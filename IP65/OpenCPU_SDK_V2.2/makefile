
#-------------------------------------------------------------------------------
# config tools, tool path
#-------------------------------------------------------------------------------
ARMTOOL_PATH=c:\progra~1\arm\adsv1_2


#-------------------------------------------------------------------------------
# config version
#-------------------------------------------------------------------------------
PLATFORM=QL
BUILD=CUSTOM
VERSION=V01


#-------------------------------------------------------------------------------
# config build dir
#-------------------------------------------------------------------------------
BUILD_DIRS=customer example

#-------------------------------------------------------------------------------
# config link libs
#-------------------------------------------------------------------------------
#LIBS=customerlib\example_customerlib.a
LIBS=


all:
	@$(MAKE) help

help:	
	@echo **********************help*******************
	@echo  please you input following command to make example
	@echo  make helloworld
	@echo  make custom
	@echo  make at
	@echo  make atc_pipe
	@echo  make gpio
	@echo  make eint
	@echo  make memory
	@echo  make pwm
	@echo  make clk
	@echo  make alert
	@echo  make keyboard
	@echo  make lcd
	@echo  make i2c
	@echo  make file
	@echo  make audio
	@echo  make transpass
	@echo  make tcpip
	@echo  make multitask
	@echo  make multitask_port
	@echo  make sms
	@echo  make call
	@echo  make headset
	@echo  make lcm_st7545t
	@echo  make lcm_uc1701
	@echo  make tcplong
	@echo  make multimemory
	@echo  make lowrateuart
	@echo  make eintcallback
	@echo  make fota         [only for FLASH 128Mb+32Mb]
	@echo  make fota_http    [only for FLASH 128Mb+32Mb]
	@echo  make tts          [only for FLASH 128Mb+32Mb]

helloworld:
	@echo *******make helloworld*********
	@echo -D__EXAMPLE_HELLWORLD__ > .\$(DEFVIATMP)
	@$(MAKE) new

custom:
	@echo *******make custom*********
	@echo -D__CUSTOMER_CODE__ > .\$(DEFVIATMP)
	@$(MAKE) new

gpio:
	@echo *******make gpio example*********
	@echo -D__EXAMPLE_GPIO__ > .\$(DEFVIATMP)
	@$(MAKE) new

eint:
	@echo *******make eint example*********
	@echo -D__EXAMPLE_EINT__ > .\$(DEFVIATMP)
	@$(MAKE) new

at:
	@echo *******make at example*********
	@echo -D__EXAMPLE_AT__ > .\$(DEFVIATMP)
	@$(MAKE) new

atc_pipe:
	@echo *******make atc_pipe example*********
	@echo -D__ATC_PIPE__ > .\$(DEFVIATMP)
	@$(MAKE) new

memory:
	@echo *******make memory example*********
	@echo -D__EXAMPLE_MEMORY__ > .\$(DEFVIATMP)
	@$(MAKE) new

pwm:
	@echo *******make pwm example*********
	@echo -D__EXAMPLE_PWM__ > .\$(DEFVIATMP)
	@$(MAKE) new


clk:
	@echo *******make clk example*********
	@echo -D__EXAMPLE_CLK__ > .\$(DEFVIATMP)
	@$(MAKE) new

alert:
	@echo *******make alert example*********
	@echo -D__EXAMPLE_ALERT__ > .\$(DEFVIATMP)
	@$(MAKE) new

keyboard:
	@echo *******make keyboard example*********
	@echo -D__EXAMPLE_KEYBOARD__ > .\$(DEFVIATMP)
	@$(MAKE) new

lcd:
	@echo *******make lcd example*********
	@echo -D__EXAMPLE_LCD__ > .\$(DEFVIATMP)
	@$(MAKE) new


i2c:
	@echo *******make i2c example*********
	@echo -D__EXAMPLE_I2C__ > .\$(DEFVIATMP)
	@$(MAKE) new	

file:
	@echo *******make file example*********
	@echo -D__EXAMPLE_FILESYSTEM__ > .\$(DEFVIATMP)
	@$(MAKE) new	

audio:
	@echo *******make audio example*********
	@echo -D__EXAMPLE_AUDIO__ > .\$(DEFVIATMP)
	@$(MAKE) new	

transpass:
	@echo *******make transpass example*********
	@echo -D__EXAMPLE_TRANSPASS__> .\$(DEFVIATMP)
	@$(MAKE) new	

tcpip:
	@echo *******make tcpip example*********
	@echo -D__EXAMPLE_TCPIP__> .\$(DEFVIATMP)
	@$(MAKE) new	

multitask:
	@echo *******make multitask example*********
	@echo -D__EXAMPLE_MULTITASK__> .\$(DEFVIATMP)
	@$(MAKE) new	
	
multitask_port:
	@echo *******make multitask_port example*********
	@echo -D__EXAMPLE_MULTITASK_PORT__> .\$(DEFVIATMP)
	@$(MAKE) new	
	
sms:
	@echo *******make sms example*********
	@echo -D__EXAMPLE_SMS__ > .\$(DEFVIATMP)
	@$(MAKE) new
	
call:
	@echo *******make call example*********
	@echo -D__EXAMPLE_CALL__ > .\$(DEFVIATMP)
	@$(MAKE) new

headset:
	@echo *******make headset example*********
	@echo -D__EXAMPLE_HEADSET__ > .\$(DEFVIATMP)
	@$(MAKE) new	
	
lcm_st7545t:
	@echo *******make lcm_st7545t example*********
	@echo -D__EXAMPLE_LCM_ST7545T__ > .\$(DEFVIATMP)
	@$(MAKE) new	

lcm_uc1701:
	@echo *******make lcm_uc1701 example*********
	@echo -D__EXAMPLE_LCM_UC1701__ > .\$(DEFVIATMP)
	@$(MAKE) new	

tcplong:
	@echo *******make tcplong example*********
	@echo -D__EXAMPLE_TCPLONG__ > .\$(DEFVIATMP)
	@$(MAKE) new	
	
multimemory:
	@echo *******make multimemory example*********
	@echo -D__EXAMPLE_MULTIMEMORY__ > .\$(DEFVIATMP)
	@$(MAKE) new	

lowrateuart:
	@echo *******make lowrateuart example*********
	@echo -D__EXAMPLE_LOWRATEUART__ > .\$(DEFVIATMP)
	@$(MAKE) new	
	
eintcallback:	
	@echo *******make eintcallback example*********
	@echo -D__EXAMPLE_EINTCALLBACK__ > .\$(DEFVIATMP)
	@$(MAKE) new	
	
fota:	
	@echo *******make fota example*********
	@echo -D__EXAMPLE_FOTA__ > .\$(DEFVIATMP)
	@$(MAKE) new		
	
fota_http:	
	@echo *******make fota_http example*********
	@echo -D__EXAMPLE_FOTA_HTTP__ > .\$(DEFVIATMP)
	@$(MAKE) new		
	
tts:	
	@echo *******make tts example*********
	@echo -D__EXAMPLE_TTS__ > .\$(DEFVIATMP)
	@$(MAKE) new		
	
include .\makefiledef

export ARMTOOL_PATH
	
