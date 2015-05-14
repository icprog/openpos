#
# Joystick driver configuration
#
menuconfig INPUT_JOYSTICK
	bool "Joysticks/Gamepads"
	help
	  If you have a joystick, 6dof controller, gamepad, steering wheel,
	  weapon control system or something like that you can say Y here
	  and the list of supported devices will be displayed. This option
	  doesn't affect the kernel.

	  Please read the file <file:Documentation/input/joystick.txt> which
	  contains more information.

if INPUT_JOYSTICK

config JOYSTICK_ANALOG
	tristate "Classic PC analog joysticks and gamepads"
	select GAMEPORT
	---help---
	  Say Y here if you have a joystick that connects to the PC
	  gameport. In addition to the usual PC analog joystick, this driver
	  supports many extensions, including joysticks with throttle control,
	  with rudders, additional hats and buttons compatible with CH
	  Flightstick Pro, ThrustMaster FCS, 6 and 8 button gamepads, or
	  Saitek Cyborg joysticks.

	  Please read the file <file:Documentation/input/joystick.txt> which
	  contains more information.

	  To compile this driver as a module, choose M here: the
	  module will be called analog.

config JOYSTICK_A3D
	tristate "Assassin 3D and MadCatz Panther devices"
	select GAMEPORT
	help
	  Say Y here if you have an FPGaming or MadCatz controller using the
	  A3D protocol over the PC gameport.

	  To compile this driver as a module, choose M here: the
	  module will be called a3d.

config JOYSTICK_ADI
	tristate "Logitech ADI digital joysticks and gamepads"
	select GAMEPORT
	help
	  Say Y here if you have a Logitech controller using the ADI
	  protocol over the PC gameport.

	  To compile this driver as a module, choose M here: the
	  module will be called adi.

config JOYSTICK_COBRA
	tristate "Creative Labs Blaster Cobra gamepad"
	select GAMEPORT
	help
	  Say Y here if you have a Creative Labs Blaster Cobra gamepad.

	  To compile this driver as a module, choose M here: the
	  module will be called cobra.

config JOYSTICK_GF2K
	tristate "Genius Flight2000 Digital joysticks and gamepads"
	select GAMEPORT
	help
	  Say Y here if you have a Genius Flight2000 or MaxFighter digitally
	  communicating joystick or gamepad.

	  To compile this driver as a module, choose M here: the
	  module will be called gf2k.

config JOYSTICK_GRIP
	tristate "Gravis GrIP joysticks and gamepads"
	select GAMEPORT
	help
	  Say Y here if you have a Gravis controller using the GrIP protocol
	  over the PC gameport.

	  To compile this driver as a module, choose M here: the
	  module will be called grip.

config JOYSTICK_GRIP_MP
	tristate "Gravis GrIP MultiPort"
	select GAMEPORT
	help
	  Say Y here if you have the original Gravis GrIP MultiPort, a hub
	  that connects to the gameport and you connect gamepads to it.

	  To compile this driver as a module, choose M here: the
	  module will be called grip_mp.

config JOYSTICK_GUILLEMOT
	tristate "Guillemot joysticks and gamepads"
	select GAMEPORT
	help
	  Say Y here if you have a Guillemot joystick using a digital
	  protocol over the PC gameport.

	  To compile this driver as a module, choose M here: the
	  module will be called guillemot.

config JOYSTICK_INTERACT
	tristate "InterAct digital joysticks and gamepads"
	select GAMEPORT
	help
	  Say Y here if you have an InterAct gameport or joystick
	  communicating digitally over the gameport.

	  To compile this driver as a module, choose M here: the
	  module will be called interact.

config JOYSTICK_SIDEWINDER
	tristate "Microsoft SideWinder digital joysticks and gamepads"
	select GAMEPORT
	help
	  Say Y here if you have a Microsoft controller using the Digital
	  Overdrive protocol over PC gameport.

	  To compile this driver as a module, choose M here: the
	  module will be called sidewinder.

config JOYSTICK_TMDC
	tristate "ThrustMaster DirectConnect joysticks and gamepads"
	select GAMEPORT
	help
	  Say Y here if you have a ThrustMaster controller using the
	  DirectConnect (BSP) protocol over the PC gameport.

	  To compile this driver as a module, choose M here: the
	  module will be called tmdc.

source "drivers/input/joystick/iforce/Kconfig"

config JOYSTICK_WARRIOR
	tristate "Logitech WingMan Warrior joystick"
	select SERIO
	help
	  Say Y here if you have a Logitech WingMan Warrior joystick connected
	  to your computer's serial port.

	  To compile this driver as a module, choose M here: the
	  module will be called warrior.

config JOYSTICK_MAGELLAN
	tristate "LogiCad3d Magellan/SpaceMouse 6dof controllers"
	select SERIO
	help
	  Say Y here if you have a Magellan or Space Mouse 6DOF controller
	  connected to your computer's serial port.

	  To compile this driver as a module, choose M here: the
	  module will be called magellan.

config JOYSTICK_SPACEORB
	tristate "SpaceTec SpaceOrb/Avenger 6dof controllers"
	select SERIO
	help
	  Say Y here if you have a SpaceOrb 360 or SpaceBall Avenger 6DOF
	  controller connected to your computer's serial port.

	  To compile this driver as a module, choose M here: the
	  module will be called spaceorb.

config JOYSTICK_SPACEBALL
	tristate "SpaceTec SpaceBall 6dof controllers"
	select SERIO
	help
	  Say Y here if you have a SpaceTec SpaceBall 2003/3003/4000 FLX
	  controller connected to your computer's serial port. For the
	  SpaceBall 4000 USB model, use the USB HID driver.

	  To compile this driver as a module, choose M here: the
	  module will be called spaceball.

config JOYSTICK_STINGER
	tristate "Gravis Stinger gamepad"
	select SERIO
	help
	  Say Y here if you have a Gravis Stinger connected to one of your
	  serial ports.

	  To compile this driver as a module, choose M here: the
	  module will be called stinger.

config JOYSTICK_TWIDJOY
	tristate "Twiddler as a joystick"
	select SERIO
	help
	  Say Y here if you have a Handykey Twiddler connected to your
	  computer's serial port and want to use it as a joystick.

	  To compile this driver as a module, choose M here: the
	  module will be called twidjoy.

config JOYSTICK_ZHENHUA
	tristate "5-byte Zhenhua RC transmitter"
	select SERIO
	help
	  Say Y here if you have a Zhen Hua PPM-4CH transmitter which is
	  supplied with a ready to fly micro electric indoor helicopters
	  such as EasyCopter, Lama, MiniCopter, DragonFly or Jabo and want
	  to use it via serial cable as a joystick.

	  To compile this driver as a module, choose M here: the
	  module will be called zhenhua.

config JOYSTICK_DB9
	tristate "Multisystem, Sega Genesis, Saturn joysticks and gamepads"
	depends on PARPORT
	help
	  Say Y here if you have a Sega Master System gamepad, Sega Genesis
	  gamepad, Sega Saturn gamepad, or a Multisystem -- Atari, Amiga,
	  Commodore, Amstrad CPC joystick connected to your parallel port.
	  For more information on how to use the driver please read
	  <file:Documentation/input/joystick-parport.txt>.

	  To compile this driver as a module, choose M here: the
	  module will be called db9.

config JOYSTICK_GAMECON
	tristate "Multisystem, NES, SNES, N64, PSX joysticks and gamepads"
	depends on PARPORT
	---help---
	  Say Y here if you have a Nintendo Entertainment System gamepad,
	  Super Nintendo Entertainment System gamepad, Nintendo 64 gamepad,
	  Sony PlayStation gamepad or a Multisystem -- Atari, Amiga,
	  Commodore, Amstrad CPC joystick connected to your parallel port.
	  For more information on how to use the driver please read
	  <file:Documentation/input/joystick-parport.txt>.

	  To compile this driver as a module, choose M here: the
	  module will be called gamecon.

config JOYSTICK_TURBOGRAFX
	tristate "Multisystem joysticks via TurboGraFX device"
	depends on PARPORT
	help
	  Say Y here if you have the TurboGraFX interface by Steffen Schwenke,
	  and want to use it with Multisystem -- Atari, Amiga, Commodore,
	  Amstrad CPC joystick. For more information on how to use the driver
	  please read <file:Documentation/input/joystick-parport.txt>.

	  To compile this driver as a module, choose M here: the
	  module will be called turbografx.

config JOYSTICK_AMIGA
	tristate "Amiga joysticks"
	depends on AMIGA
	help
	  Say Y here if you have an Amiga with a digital joystick connected
	  to it.

	  To compile this driver as a module, choose M here: the
	  module will be called amijoy.

config JOYSTICK_JOYDUMP
	tristate "Gameport data dumper"
	select GAMEPORT
	help
	  Say Y here if you want to dump data from your joystick into the system
	  log for debugging purposes. Say N if you are making a production
	  configuration or aren't sure.

	  To compile this driver as a module, choose M here: the
	  module will be called joydump.

config JOYSTICK_XPAD
	tristate "X-Box gamepad support"
	depends on USB_ARCH_HAS_HCD
	select USB
	help
	  Say Y here if you want to use the X-Box pad with your computer.
	  Make sure to say Y to "Joystick support" (CONFIG_INPUT_JOYDEV)
	  and/or "Event interface support" (CONFIG_INPUT_EVDEV) as well.

	  For information about how to connect the X-Box pad to USB, see
	  <file:Documentation/input/xpad.txt>.

	  To compile this driver as a module, choose M here: the
	  module will be called xpad.

config JOYSTICK_XPAD_FF
	bool "X-Box gamepad rumble support"
	depends on JOYSTICK_XPAD && INPUT
	select INPUT_FF_MEMLESS
	---help---
	  Say Y here if you want to take advantage of xbox 360 rumble features.

config JOYSTICK_XPAD_LEDS
	bool "LED Support for Xbox360 controller 'BigX' LED"
	depends on JOYSTICK_XPAD && (LEDS_CLASS=y || LEDS_CLASS=JOYSTICK_XPAD)
	---help---
	  This option enables support for the LED which surrounds the Big X on
	  XBox 360 controller.

config JOYSTICK_WALKERA0701
	tristate "Walkera WK-0701 RC transmitter"
	depends on HIGH_RES_TIMERS && PARPORT
	help
	  Say Y or M here if you have a Walkera WK-0701 transmitter which is
	  supplied with a ready to fly Walkera helicopters such as HM36,
	  HM37, HM60 and want to use it via parport as a joystick. More
	  information is available: <file:Documentation/input/walkera0701.txt>

	  To compile this driver as a module, choose M here: the
	  module will be called walkera0701.

config JOYSTICK_MAPLE
	tristate "Dreamcast control pad"
	depends on MAPLE
	help
	  Say Y here if you have a SEGA Dreamcast and want to use your
	  controller as a joystick.

	  Most Dreamcast users will say Y.

	  To compile this as a module choose M here: the module will be called
	  maplecontrol.

endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              10

dir
1524
http://172.16.250.7/svn/YF-110510-01/soft/trunk/kernel/drivers/video/kyro
http://172.16.250.7/svn/YF-110510-01



2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu














260be20b-3001-0010-90b4-153382fa7df0

STG4000VTG.c
file




2013-08-27T09:45:20.035254Z
5f96db8b234b2baa8111782267835fd3
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















4649

STG4000OverlayDevice.c
file




2013-08-27T09:45:20.035254Z
b7c813ffc47a9bfb48ecc4d05ba0dcac
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















15651

STG4000Reg.h
file




2013-08-27T09:45:20.036254Z
2071284048b08303bb111d3c87a09440
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















10861

STG4000InitDevice.c
file




2013-08-27T09:45:20.036254Z
2cef193413204636979b5fed80452136
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















9799

fbdev.c
file




2013-08-27T09:45:20.036254Z
e0dd1a1b603d586fe71d65f5eda51169
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















21070

STG4000Interface.h
file




2013-08-27T09:45:20.040254Z
ce9d3b05a8770234890bf05ffad84061
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















1897

STG4000Ramdac.c
file




2013-08-27T09:45:20.040254Z
09cc65a2c498adad628589f1ae6936f0
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















3931

Makefile
file




2013-08-27T09:45:20.040254Z
391abd3e1a219d18158a80063c567ef1
2013-01-25T07:39:01.400462Z
1063
sand\x5czengshu





















185

 