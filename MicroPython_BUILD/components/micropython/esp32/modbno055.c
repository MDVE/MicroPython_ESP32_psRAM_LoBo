/*
 * This file might be part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 MDVE LLC (https://github.com/MDVE)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * blah blah blah
 */

#include "sdkconfig.h"

#ifdef CONFIG_MICROPY_USE_BNO055

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "extmod/bno055/bno055.h"

#include "py/nlr.h"
#include "py/runtime.h"
#include "modmachine.h"
#include "mphalport.h"


typedef struct _bno055_obj_t {
    mp_obj_base_t base;
    struct bno055_t *imu;
    int i2caddr;
} bno055_obj_t;

const mp_obj_type_t bno055_type;



//--------------------------------------
STATIC int checkIMU(bno055_obj_t *self)
{
	if (self->imu == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "IMU suspended"));
   }
	return 0;
}


//-------------------------------------------------------------------------------------
STATIC void bno055_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    bno055_obj_t *self = self_in;

    if (self->imu == NULL) {
    	mp_printf(print, "IMU [%d]( Suspended )\n", self->i2caddr);
    	return;
    }
    
	 mp_printf(print, "IMU [%d]( Unknown )\n", self->i2caddr);
}


//------------------------------------------------------------------------------------------------------------
STATIC mp_obj_t bno055_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args)
{
	enum { ARG_i2caddr, ARG_sdapin, ARG_sclpin, ARG_intpin, ARG_resetpin, ARG_i2cspeed };

    const mp_arg_t bno055_init_allowed_args[] = {
			{ MP_QSTR_bno055_i2caddr,         	MP_ARG_KW_ONLY  | MP_ARG_INT,  {.u_int = 40} },
			{ MP_QSTR_bno055_sdapin,         	MP_ARG_KW_ONLY  | MP_ARG_INT,  {.u_int = 4} },
			{ MP_QSTR_bno055_sclpin,         	MP_ARG_KW_ONLY  | MP_ARG_INT,  {.u_int = 16} },
			{ MP_QSTR_bno055_intpin,         	MP_ARG_KW_ONLY  | MP_ARG_INT,  {.u_int = 17} },
			{ MP_QSTR_bno055_resetpin,         	MP_ARG_KW_ONLY  | MP_ARG_INT,  {.u_int = 5} },
			{ MP_QSTR_bno055_i2cspeed,         	MP_ARG_KW_ONLY  | MP_ARG_INT,  {.u_int = 400000} },
	};
	mp_arg_val_t args[MP_ARRAY_SIZE(bno055_init_allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(bno055_init_allowed_args), bno055_init_allowed_args, args);

    // Setup the bno055 object
    bno055_obj_t *self = m_new_obj(bno055_obj_t );

    // === allocate IMU memory ===
    self->imu = malloc(sizeof(struct bno055_t));
    if (self->imu == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "Error allocating memory"));
    }
    memset(self->imu, 0, sizeof(struct bno055_t));
    
    bno055_init(self->imu);
    
    // Populate settings
    self->i2caddr = args[ARG_i2caddr].u_int;

    self->base.type = &bno055_type;

    return MP_OBJ_FROM_PTR(self);
}

//----------------------------------------------
STATIC mp_obj_t bno055_op_status(mp_obj_t self_in)
{
    bno055_obj_t *self = self_in;
    checkIMU(self);

	 mp_obj_t tuple[2];

    tuple[0] = mp_obj_new_int(-1);
	 tuple[1] = mp_obj_new_str("STATUS", 6, 0);

	return mp_obj_new_tuple(2, tuple);
}
MP_DEFINE_CONST_FUN_OBJ_1(bno055_status_obj, bno055_op_status);

//--------------------------------------------
STATIC mp_obj_t bno055_op_suspend(mp_obj_t self_in)
{
    bno055_obj_t *self = self_in;
    checkIMU(self);

    vTaskDelay(100 / portTICK_RATE_MS);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(bno055_suspend_obj, bno055_op_suspend);

//--------------------------------------------
STATIC mp_obj_t bno055_op_free(mp_obj_t self_in)
{
    bno055_obj_t *self = self_in;
	if (self->imu) {
    	free(self->imu);
    	self->imu = NULL;
        return mp_const_true;
    }
    return mp_const_false;
}
MP_DEFINE_CONST_FUN_OBJ_1(bno055_free_obj, bno055_op_free);


//=========================================================
STATIC const mp_rom_map_elem_t bno055_locals_dict_table[] = {
	    { MP_ROM_QSTR(MP_QSTR_bno055_status),		(mp_obj_t)&bno055_status_obj },
	    { MP_ROM_QSTR(MP_QSTR_bno055_suspend),		(mp_obj_t)&bno055_suspend_obj },
	    { MP_ROM_QSTR(MP_QSTR_bno055_free),		(mp_obj_t)&bno055_free_obj },
};
STATIC MP_DEFINE_CONST_DICT(bno055_locals_dict, bno055_locals_dict_table);

//===============================
const mp_obj_type_t bno055_type = {
    { &mp_type_type },
    .name = MP_QSTR_Bno055,
    .print = bno055_print,
    .make_new = bno055_make_new,
    .locals_dict = (mp_obj_dict_t*)&bno055_locals_dict,
};

#endif

