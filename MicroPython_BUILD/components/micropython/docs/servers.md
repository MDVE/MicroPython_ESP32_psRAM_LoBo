# TELNET & FTP servers

### Telnet server

Telnet server is implemented as ESP32 task and runs in background (when started).

When user is connected, REPL is paused and resumes after the user disconnets.

User can be disconnected from REPL by pressing Ctrl_T


| Method  | Notes |
| - | - |
| network.telnet.start([user="micro",password="python", timeout=300]) | Start Telnet server. The arguments are optional, if not set the dafaults are used. |
| network.telnet.pause() | Pause Telnet server |
| network.telnet.resume() | Resume Telnet server |
| network.telnet.stop() | Stop Telnet server. The ESP32 task is terminated, all memory is freed. |
| network.telnet.status() | Get the current Telnet server status. The status is returned as tuple of status numeric value and string description |
| network.telnet.stack() | Get the maximum amount of stack used by the Telnet task. The stack size is set at compile time and can be changed in *mpthreadport.h* |

### Ftp server

Ftp server is implemented as ESP32 task and runs in background (when started).


| Method  | Notes |
| - | - |
| network.ftp.start([user="micro",password="python",buffsize=1024,timeout=300]) | Start FTP server. The arguments are optional, if not set the dafaults are used. *Buffsize* sets the transfer buffer size, larger buffer enables faster transfers, but uses more memory. |
| network.ftp.pause() | Pause Ftp server. Only when no client is connected. |
| network.ftp.resume() | Resume Ftp server. Only when no client is connected. |
| network.ftp.stop() | Stop Ftp server. Only when no client is connected. The ESP32 task is terminated, all memory is freed. |
| network.ftp.status() | Get the current Ftp server status. The status is returned as tuple of numeric value and string description of Command and Data channels. |
| network.ftp.stack() | Get the maximum amount of stack used by the Ftp task. The stack size is set at compile time and can be changed in *mpthreadport.h* |

## Notes

* Only one connection to FTP server at the time can be used
* **Passive** transfer mode must be used
* Only one data connection can be used, make shure your FTP client does not try to establish multiple data connections
* No encription is allowed, configure your client to use plain FTP (insecure). That shouldn't be an issue, you will mostly use FTP server in local network.

---

Example of FileZilla settings:

![General Settings](https://raw.githubusercontent.com/loboris/MicroPython_ESP32_psRAM_LoBo/master/Documents/FileZilla_1.png)
![Transfer Settings](https://raw.githubusercontent.com/loboris/MicroPython_ESP32_psRAM_LoBo/master/Documents/FileZilla_2.png)
