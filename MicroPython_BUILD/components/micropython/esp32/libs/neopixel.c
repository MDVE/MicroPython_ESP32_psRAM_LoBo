
#include "driver/gpio.h"
#include "driver/rmt.h"
#include <math.h>

#include "libs/neopixel.h"


static xSemaphoreHandle neopixel_sem = NULL;
static intr_handle_t rmt_intr_handle = NULL;
static rmt_channel_t RMTchannel = RMT_CHANNEL_0;
static uint16_t neopixel_pos, neopixel_buf_len, neopixel_half, neopixel_bufIsDirty;
static pixel_settings_t *neopixel_px;
static rmt_item32_t RMT_Items[MAX_PULSES*2+1];
static uint8_t *neopixel_buffer = NULL;
static uint8_t neopixel_bpp;

/**
 * Set two levels of RMT output to the Neopixel value for a "1".
 * This is:
 *  * a logic 1 for 0.7us
 *  * a logic 0 for 0.6us
 */
//--------------------------------------------------------------------
static void neopixel_mark(rmt_item32_t *pItem, pixel_settings_t *px) {
	pItem->level0    = px->timings.mark.level0;
	pItem->duration0 = px->timings.mark.duration0;
	pItem->level1    = px->timings.mark.level1;
	pItem->duration1 = px->timings.mark.duration1;
}

/**
 * Set two levels of RMT output to the Neopixel value for a "0".
 * This is:
 *  * a logic 1 for 0.35us
 *  * a logic 0 for 0.8us
 */
//---------------------------------------------------------------------
static void neopixel_space(rmt_item32_t *pItem, pixel_settings_t *px) {
	pItem->level0    = px->timings.space.level0;
	pItem->duration0 = px->timings.space.duration0;
	pItem->level1    = px->timings.space.level1;
	pItem->duration1 = px->timings.space.duration1;
}

//--------------------------------------------------------------------
static void rmt_terminate(rmt_item32_t *pItem, pixel_settings_t *px) {
	pItem->level0    = px->timings.reset.level0;
	pItem->duration0 = px->timings.reset.duration0;
	pItem->level1    = px->timings.reset.level1;
	pItem->duration1 = px->timings.reset.duration1;
}

//----------------------------------------
uint8_t offset_color(char o, pixel_t *p) {
	switch(o) {
		case 'R': return p->red;
		case 'B': return p->green;
		case 'G': return p->blue;
		case 'W': return p->white;
	}
	return 0;
}

//----------------------------------------------------------------
void np_set_color_order(pixel_settings_t *px, pixel_order_t order)
{
	px->white_offset = (order >> 6) & 0b11;
	px->red_offset   = (order >> 4) & 0b11;
	px->green_offset = (order >> 2) & 0b11;
	px->blue_offset  = order & 0b11;
}

//------------------------------------------------------------------------------------------------------------------
void np_set_pixel_color(pixel_settings_t *px, uint16_t idx, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
	px->pixels[idx].red   = (uint16_t)(red * px->brightness) / 255;
	px->pixels[idx].green = (uint16_t)(green * px->brightness) / 255;
	px->pixels[idx].blue  = (uint16_t)(blue * px->brightness) / 255;
	px->pixels[idx].white = (uint16_t)(white * px->brightness) / 255;
}

//---------------------------------------------------------------------------
void np_set_pixel_color32(pixel_settings_t *px, uint16_t idx, uint32_t color)
{
	px->pixels[idx].red   = (uint16_t)(((color >> 24) & 0xFF) * px->brightness) / 255;
	px->pixels[idx].green = (uint16_t)(((color >> 16) & 0xFF) * px->brightness) / 255;
	px->pixels[idx].blue  = (uint16_t)(((color >> 8) & 0xFF) * px->brightness) / 255;
	px->pixels[idx].white = (uint16_t)((color && 0xFF) * px->brightness) / 255;
}

//------------------------------------------------------------------------------------------------------------
void np_set_pixel_color_hsb(pixel_settings_t *px, uint16_t idx, float hue, float saturation, float brightness)
{
	uint32_t color = hsb_to_rgb(hue, saturation, brightness);
	np_set_pixel_color32(px, idx, color);
}

//----------------------------------------------------------------------------------------------------------------------
void np_get_pixel_color(pixel_settings_t *px, uint16_t idx, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *white)
{
	*red = px->pixels[idx].red;
	*green = px->pixels[idx].green;
	*blue = px->pixels[idx].blue;
	*white = px->pixels[idx].white;
}

uint32_t np_get_pixel_color32(pixel_settings_t *px, uint16_t idx)
{
	return (uint32_t)((px->pixels[idx].red << 24) || (px->pixels[idx].green << 16) || (px->pixels[idx].blue << 8) || (px->pixels[idx].white));
}



//-------------------------------
static void copyToRmtBlock_half()
{
	// This fills half an RMT block
	// When wrap around is happening, we want to keep the inactive half of the RMT block filled
	uint16_t i, offset, len, byteval;

	offset = neopixel_half * MAX_PULSES;
	neopixel_half = !neopixel_half;

	len = neopixel_buf_len - neopixel_pos;
	if (len > (MAX_PULSES / 8)) len = (MAX_PULSES / 8);

	if (!len) {
	if (!neopixel_bufIsDirty) return;
		// Clear the channel's data block and return
		for (i = 0; i < MAX_PULSES; i++) {
			RMTMEM.chan[RMTchannel].data32[i + offset].val = 0;
		}
		neopixel_bufIsDirty = 0;
		return;
	}
	neopixel_bufIsDirty = 1;

	rmt_item32_t * pCurrentItem;
	if (neopixel_half) pCurrentItem = RMT_Items + (sizeof(rmt_item32_t) * MAX_PULSES);
	else pCurrentItem = RMT_Items;

	// Populate RMT bit buffer
	for (i = 0; i < len; i++) {
		byteval = neopixel_buffer[i+neopixel_pos];

		// Shift bits out, MSB first, setting RMTMEM.chan[n].data32[x] to the rmtPulsePair value corresponding to the buffered bit value
		for (int j=7; j>=0; j--) {
			if (byteval & (1<<j)) neopixel_mark(pCurrentItem, neopixel_px);
			else neopixel_space(pCurrentItem, neopixel_px);

			RMTMEM.chan[RMTchannel].data32[i * 8 + offset + (7-j)].val = pCurrentItem->val;
			pCurrentItem++;
		}
		if (i + neopixel_pos == neopixel_buf_len - 1) {
			rmt_terminate(pCurrentItem, neopixel_px);
			RMTMEM.chan[RMTchannel].data32[i * 8 + offset + 7].val = pCurrentItem->val;
		}
	}
	// Clear the remainder of the channel's data not set above
	for (i *= 8; i < MAX_PULSES; i++) {
		RMTMEM.chan[RMTchannel].data32[i + offset].val = 0;
	}

	neopixel_pos += len;
	return;
}


//-------------------------------------------------------
static void IRAM_ATTR neopixel_handleInterrupt(void *arg)
{
  portBASE_TYPE taskAwoken = 0;

  if (RMT.int_st.ch0_tx_thr_event) {
    copyToRmtBlock_half();
    RMT.int_clr.ch0_tx_thr_event = 1;
  }
  else if (RMT.int_st.ch0_tx_end && neopixel_sem) {
    xSemaphoreGiveFromISR(neopixel_sem, &taskAwoken);
    RMT.int_clr.ch0_tx_end = 1;
  }

  return;
}

//---------------------------------------------------
int neopixel_init(int gpioNum, rmt_channel_t channel)
{
	RMTchannel = channel;

	DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_RMT_CLK_EN);
	DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_RMT_RST);

	esp_err_t res = rmt_set_pin(RMTchannel, RMT_MODE_TX, (gpio_num_t)gpioNum);
	if (res != ESP_OK) return res;

	RMT.apb_conf.fifo_mask = 1;						//enable memory access, instead of FIFO mode.
	RMT.apb_conf.mem_tx_wrap_en = 1;				//wrap around when hitting end of buffer
	RMT.conf_ch[channel].conf0.div_cnt = DIVIDER;
	RMT.conf_ch[channel].conf0.mem_size = 1;
	RMT.conf_ch[channel].conf0.carrier_en = 0;
	RMT.conf_ch[channel].conf0.carrier_out_lv = 1;
	RMT.conf_ch[channel].conf0.mem_pd = 0;

	RMT.conf_ch[channel].conf1.rx_en = 0;
	RMT.conf_ch[channel].conf1.mem_owner = 0;
	RMT.conf_ch[channel].conf1.tx_conti_mode = 0;	//loop back mode.
	RMT.conf_ch[channel].conf1.ref_always_on = 1;	// use apb clock: 80M
	RMT.conf_ch[channel].conf1.idle_out_en = 1;
	RMT.conf_ch[channel].conf1.idle_out_lv = 0;

	RMT.tx_lim_ch[RMTchannel].limit = MAX_PULSES;
	RMT.int_ena.ch0_tx_thr_event = 1;
	RMT.int_ena.ch0_tx_end = 1;

	res = esp_intr_alloc(ETS_RMT_INTR_SOURCE, 0, neopixel_handleInterrupt, NULL, &rmt_intr_handle);
	if (res != ESP_OK) return res;

	if (neopixel_sem == NULL) {
		neopixel_sem = xSemaphoreCreateBinary();
		if (neopixel_sem == NULL) return ESP_FAIL;
	}

	return ESP_OK;
}

//-----------------------------------------
void neopixel_deinit(rmt_channel_t channel)
{
	if (channel != RMTchannel) return;

	rmt_set_rx_intr_en(channel, 0);
    rmt_set_err_intr_en(channel, 0);
    rmt_set_tx_intr_en(channel, 0);
    rmt_set_tx_thr_intr_en(channel, 0, 0xffff);

    if (rmt_intr_handle) esp_intr_free(rmt_intr_handle);

    if (neopixel_sem) {
		vSemaphoreDelete(neopixel_sem);
		neopixel_sem = NULL;
	}
}

//--------------------------------
void np_show(pixel_settings_t *px)
{
	uint16_t i;
	uint8_t bpp = px->nbits/8;
	neopixel_buf_len = (px->pixel_count * bpp);
	neopixel_buffer = (uint8_t *) malloc(neopixel_buf_len);

	for (i = 0; i < px->pixel_count; i++) {
		neopixel_buffer[(i * bpp) + 0] = offset_color(px->color_order[0], &px->pixels[i]);
		neopixel_buffer[(i * bpp) + 1] = offset_color(px->color_order[1], &px->pixels[i]);
		neopixel_buffer[(i * bpp) + 2] = offset_color(px->color_order[2], &px->pixels[i]);
		if (bpp > 3) neopixel_buffer[(i * bpp) + 3] = offset_color(px->color_order[3], &px->pixels[i]);
	}

	neopixel_pos = 0;
	neopixel_half = 0;
	neopixel_bpp = bpp;
	neopixel_px = px;
	neopixel_half = 0;

	copyToRmtBlock_half();

	if (neopixel_pos < neopixel_buf_len) {
		// Fill the other half of the buffer block
		copyToRmtBlock_half();
	}

	RMT.conf_ch[RMTchannel].conf1.mem_rd_rst = 1;
	RMT.conf_ch[RMTchannel].conf1.tx_start = 1;

	xSemaphoreTake(neopixel_sem, portMAX_DELAY);
	free(neopixel_buffer);
}

//==================================================================================================

//---------------------------------
void np_clear(pixel_settings_t *px)
{
	for(size_t i = 0; i < px->pixel_count; ++i) {
		np_set_pixel_color(px, i, 0, 0, 0, 0);
	}
}

//------------------------------------
static float Min(double a, double b) {
	return a <= b ? a : b;
}

//------------------------------------
static float Max(double a, double b) {
	return a >= b ? a : b;
}

//-------------------------------------------------------------------
void rgb_to_hsb( uint32_t color, float *hue, float *sat, float *bri )
{
	float delta, min;
	float h = 0, s, v;
	uint8_t red = (color >> 24) & 0xFF;
	uint8_t green = (color >> 16) & 0xFF;
	uint8_t blue = (color >> 8) & 0xFF;

	min = Min(Min(red, green), blue);
	v = Max(Max(red, green), blue);
	delta = v - min;

	if (v == 0.0) s = 0;
	else s = delta / v;

	if (s == 0)	h = 0.0;
	else
	{
		if (red == v)
			h = (green - blue) / delta;
		else if (green == v)
			h = 2 + (blue - red) / delta;
		else if (blue == v)
			h = 4 + (red - green) / delta;

		h *= 60;

		if (h < 0.0) h = h + 360;
	}

	*hue = h;
	*sat = s;
	*bri = v / 255;
}

//------------------------------------------------------------
uint32_t hsb_to_rgb(float _hue, float _sat, float _brightness)
{
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;

	if (_sat == 0.0) {
		red = _brightness;
		green = _brightness;
		blue = _brightness;
	}
	else {
		if (_hue >= 360.0) _hue = fmod(_hue, 360);

		int slice = (int)(_hue / 60.0);
		float hue_frac = (_hue / 60.0) - slice;

		float aa = _brightness * (1.0 - _sat);
		float bb = _brightness * (1.0 - _sat * hue_frac);
		float cc = _brightness * (1.0 - _sat * (1.0 - hue_frac));

		switch(slice) {
			case 0:
				red = _brightness;
				green = cc;
				blue = aa;
				break;
			case 1:
				red = bb;
				green = _brightness;
				blue = aa;
				break;
			case 2:
				red = aa;
				green = _brightness;
				blue = cc;
				break;
			case 3:
				red = aa;
				green = bb;
				blue = _brightness;
				break;
			case 4:
				red = cc;
				green = aa;
				blue = _brightness;
				break;
			case 5:
				red = _brightness;
				green = aa;
				blue = bb;
				break;
			default:
				red = 0.0;
				green = 0.0;
				blue = 0.0;
				break;
		}
	}

	return ((uint8_t)(red * 255.0) << 24) | ((uint8_t)(green * 255.0) << 16) | ((uint8_t)(blue * 255.0) << 8);
}

