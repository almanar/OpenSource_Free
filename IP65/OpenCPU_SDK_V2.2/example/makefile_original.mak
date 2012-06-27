

INCLUDES =  -I .\\  -I..\inc\\  -I..\qlinc\\

OBJS= example_at.o example_gpio.o example_eint.o example_memory.o example_pwm.o  \
			example_clk.o example_alert.o  example_keyboard.o example_lcd.o example_i2c.o example_file.o \
			example_audio.o example_transpass.o example_tcpip.o \
			example_multitask.o resource_audio.o example_sms.o example_headset.o \
			example_lcm_st7545t.o example_multitask_port.o example_lcm_uc1701.o example_tcplong.o example_multimemory.o \
			example_lowrateuart.o example_eintcallback.o example_fota.o example_helloWorld.o\
			example_fota_http.o\
			example_tts.o atc_pipe.o example_call.o example_float_test.o example_led.o

all:listobj $(OBJS)


include ..\makefiledef
