Based on program by:

Michael McElligott
-----------------
http://mylcd.sourceforge.net
okio@users.sourceforge.net

Requirements:

- Follow "Device Driver Installation" instructions to install libusb-win32 driver from here: http://sourceforge.net/apps/trac/libusb-win32/wiki
NOTE: To do the above you need the USB VID/PID of your USB dongle available under Windows Device Manager->(Device)->Properties->Details->Hardware ID's.  For example my Garmin and mini Suunto VID/PIDS were both 0FCF/1008.
- If you have factory device driver for USB dongle already installed, you will have to update it's driver to the new libusb-win32 one you created above.  This is also done in Windows Device Manager.
- If the program opens and then closes before showing the heart rate, simply run it again and it should sync up your HRM strap with the dongle and program.

Changes:

- Added zones, calories, burn rate, workout duration, workout average etc.
- Added settings.ini configuration to change background image (should be a 320x240 png image) and personal data for calculations.
- Added export of workout data to CSV file if export option is enabled in settings.ini
- Added colour highlighting of zone for quick visual update of how you are doing:
	- Rest: Blue
	- Warmup: Violet
	- Fat Burning: Light Green
	- Aerobic: Green
	- Anaerobic: Yellow
	- VO2 Max: Orange
	- Max HRT: Red

NOTES:

- Function that calculates the clients maximum HRT based on function from wikipedia: HRmax = 191.5 - (0.007 × age^2)  See: http://en.wikipedia.org/wiki/Heart_rate#Formula
- Function that sets the various zones based on the client's calculated maximum HRT.  See http://en.wikipedia.org/wiki/File:Exercise_zones.png
- Function to calculate the calories used since the last HRT reading and track the total taken from "Prediction of energy expenditure from heart rate monitoring during submaximal exercise", L. R. KEYTEL, J. H. GOEDECKE, T. D. NOAKES, H. HIILOSKORPI, R. LAUKKANEN, L. VAN DER MERWE, & E. V. LAMBERT that was published in the Journal of Sports Sciences.
Male: ((-55.0969 + (0.6309 x HR) + (0.1988 x W) + (0.2017 x A))/4.184) x 60 x T
Female: ((-20.4022 + (0.4472 x HR) - (0.1263 x W) + (0.074 x A))/4.184) x 60 x T
HR = Heart rate (in beats/minute) - using average HR for this so it could go up and down as workout changes.
W = Weight (in kilograms)
A = Age (in years)
T = Exercise duration time (in hours)

HIIT:

Example programs - Enter settings into ini file passed into app.

Progressive:
Week  Warmup	  Work Int    Recovery	  Repeat	 Cooldown    Total Workout
                  (Max)     (60-70% MHR)
 1	  5 min.	  1 min.	  4 min.	  2 times	  5 min.	  20 min.
 2	  5 min.	  1 min.	  4 min.	  3 times	  5 min.	  25 min.
 3	  5 min.	  1 min.	  4 min.	  4 times	  5 min.	  30 min.
 4	  5 min.	  1.5 min.	  4 min.	  2 times	  5 min.	  21 min.
 5	  5 min.	  1.5 min.	  4 min.	  3 times	  5 min.	  26.5 min.
 6	  5 min.	  1.5 min.	  4 min.	  4 times	  5 min.	  32 min.
 7	  5 min.	  2 min.	  5 min.	  3 times	  5 min.	  31 min.
 8	  5 min.	  2 min.	  5 min.	  4 times	  5 min.	  38 min.

All In: 300s warm-up, 30s max, 90s recovery, repeat 8, 300s cooldown, total time 26 mins.

Paul Rimmer
http://www.ronin-tech.com/Content/pid=36.html