libpinproc installation notes:

The install process should have created the following subdirectories:
.\FTDI_2.xx.yy (includes the USB driver)
.\bin (includes pinprocfw.exe)
.\ext\python (includes a setup file for the pinproc python extension)

*** USB Driver ***
The FTDI driver should have been installed automatically during the libpinproc installation.  If it did not, manually run .\FTDI_2.xx.yy\DPInst.exe.  This is necessary to communicate with the P-ROC over USB.

*** pinprocfw.exe ***
This application can be used to update the P-ROC's firmware when updates are made
available on www.pinballcontrollers.com as follows:
.\bin\pinprocfw.exe <new firmware file>

*** pinproc python extension ***
If you're planning on writing or using Python scripts to communicate with your P-ROC, and you already have Python 2.6 installed, execute this setup file to install the pinproc python extension.  Note - this extension has only been tested with Python 2.6.
