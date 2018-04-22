
// anthrm - http://mylcd.sourceforge.net/
// An LCD framebuffer and text rendering API
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2011  Michael McElligott
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this library; if not, write to the Free
//  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#include <wchar.h>
#include <windows.h>
#include "mylcdsetup.h"

#include "garminhr.h"
#include "console.h"


#define G19CB           (0)
#define RELEASE         (0)


#if !RELEASE
#include <conio.h>
#endif

#if G19CB
#include "../mylcd/src/g19/g19.h"
#endif


// when trying each of the ant+ keys
// each key is tried CONNECTION_TIMEOUT times before moving on to the next key
// then repeated CONNECTION_RETRIES times
#define CONNECTION_TIMEOUT  (2)     // per key retry on failure
#define CONNECTION_RETRIES  (5)     // all key retry
#define LIBUSB_DISCONNECT   (-5)
#define LIBUSB_TIMEOUT      (-116)

#define MAX_STR_PARAM_LEN               256
#define SETTINGS                        "settings.ini"
#define MALE                            "M"
#define SEX_LENGTH                      2
#define WORKOUT_CSV_FILENAME            "_workout_%y-%m-%d_%H-%M-%S.csv"
#define CSV_FILE_HEADER                 "Reading,Time(s),HRT,Calories,Avg HRT,Max HRT,Min HRT\n"
#define WORKOUT_DURATION_LEN            20
#define MAX_WORKOUT_CSV_FILENAME_LENGTH 40
#define ROW_OFFSET                      16
#define ROW_1_TOP_OFFSET                0
#define ROW_2_TOP_OFFSET                (ROW_1_TOP_OFFSET+ROW_OFFSET)
#define ROW_3_TOP_OFFSET                (ROW_2_TOP_OFFSET+ROW_OFFSET)
#define ROW_1_BOTTOM_OFFSET             190
#define ROW_2_BOTTOM_OFFSET             (ROW_1_BOTTOM_OFFSET+ROW_OFFSET)
#define ROW_3_BOTTOM_OFFSET             (ROW_2_BOTTOM_OFFSET+ROW_OFFSET)
#define SECONDS_IN_MINUTE               60
#define MINUTES_IN_HOUR                 60
#define SECONDS_IN_HOUR                 (SECONDS_IN_MINUTE * MINUTES_IN_HOUR)
#define MS_IN_SECOND                    1000
#define MS_IN_MINUTE                    (MS_IN_SECOND * SECONDS_IN_MINUTE)
#define MS_IN_HOUR                      (MS_IN_MINUTE * MINUTES_IN_HOUR)
#define SCALE_OFFSET                    20
#define SCALE_WIDTH                     5

#define COLOUR_RED          0xFF0000
#define COLOUR_ORANGE       0xFF9E00
#define COLOUR_YELLOW       0xFFFF00
#define COLOUR_GREEN        0x00FF00
#define COLOUR_LIGHT_GREEN  0x99FF99
#define COLOUR_VIOLET       0x00FFFF   
#define COLOUR_BLUE         0x0000FF
#define COLOUR_PURPLE       0x9B30FF
#define COLOUR_WHITE        0xFFFFFF

#define MAX_ZONE_STR_LENGTH     20
#define ZONE_HIGHLIGHT_OFFSET   2

#define DANGER_TAG          "Danger!";
#define VO2MAX_TAG          "V02 Max";
#define ANAEROBIC_TAG       "Anaerobic";
#define AEROBIC_TAG         "Aerobic";
#define FATBURNING_TAG      "Fat Burning";
#define WARMUP_TAG          "Warm up";
#define REST_TAG            "Rest";

#define COLOUR_DANGER       COLOUR_RED
#define COLOUR_VO2MAX       COLOUR_ORANGE
#define COLOUR_ANAEROBIC    COLOUR_YELLOW
#define COLOUR_AEROBIC      COLOUR_GREEN
#define COLOUR_FATBURNING   COLOUR_LIGHT_GREEN
#define COLOUR_WARMUP       COLOUR_VIOLET
#define COLOUR_REST         COLOUR_BLUE

#define HIIT_BEGIN          1
#define HIIT_WARMUP         2
#define HIIT_GIVER          3
#define HIIT_REST           4
#define HIIT_COOLDOWN       5
#define HIIT_DONE           6

#define HIIT_WARMUP_TAG     "Warm-up"
#define HIIT_MAX_TAG        "Max!"
#define HIIT_REST_TAG       "Rest"
#define HIIT_COOLIT_TAG     "Cooling"
#define HIIT_DONE_TAG       "Done"

static char hiit_zone[MAX_STR_PARAM_LEN];

/*
 * Configuration items read from settings file
 */
static wchar_t name[MAX_STR_PARAM_LEN];
static unsigned int age=0, weight=0, height_f=0, export_to_csv=1;
static unsigned int intervals=0, warmup=0, allout=0, rest=0, cooldown=0;
unsigned int hiit_on=0, curr_interval=0;
// Height in inches with 1 decimal place allowed eg. 8.5, more palces will be ignored
static float height_i;
// Sex of person working out "M"ale or "F"emale
static char sex[SEX_LENGTH];
// Path to the background image of the window
static wchar_t background[MaxPath];

/*
 * Values calculated using wikipedia defined formulas/charts.
 * See http://en.wikipedia.org/wiki/Heart_rate
 */
static float maxhrt, vo2_rate, anaerobic_rate, aerobic_rate, fat_burn_rate, warm_up_rate;
static float danger_ticks=0, vo2_ticks=0, anaerobic_ticks=0, aerobic_ticks=0, fat_burn_ticks=0, warm_up_ticks=0, idle_ticks=0;
/*
 * Every time we receive a valid HR reading we track the total beats and number of readings
 * to calculate the total average heart rate for the workout.                              
 */
static long long total_beats=0, num_readings=0;

/*
 * Total of all the calorie slices calculated for each reading duration.
 */
static double calories=0;

/*
 * Time in seconds used to track workout duration
 */
static long long last_time, start_time, elapsed_time, hiit_next_eta;

// Time with ms resolution for calculating time since last calorie calculation
double start_time_ms, last_time_ms;

// Time since the last valid heart rate leading in seconds, typically 4Hz frequency
static double et_hours, et_secs, et_mins, et_ms;
//static double et_hours, et_ms;

// We need to know if this is the first reading as we won't have a prior timestamp for it
static unsigned int first_reading = 1;

// We use different calorie calculations if sex is Male vs female
static unsigned int male = 1;

// Time program has run in hours:mins:secs shown in window
static char workout_duration[WORKOUT_DURATION_LEN];
// Workout CSV data Filename using unique timestamp
static char workout_csv_filename[MAX_WORKOUT_CSV_FILENAME_LENGTH];
static char data_path[MAX_STR_PARAM_LEN];
static char workout_csv_filename2[MAX_STR_PARAM_LEN+MAX_WORKOUT_CSV_FILENAME_LENGTH];
// File pointer to the workout export file in CSV format
static FILE *csv_fp;

static int kb_hit ()
{
#if RELEASE
    return 0;
#else
    return kbhit();
#endif
}

static int getRunState (THR *hr)
{
    return hr->cstates->running;
}

static void setRunState (THR *hr, const int state)
{
    hr->cstates->running = state;
}

static int imageAdd (TIMAGES *img, const int imgIdx, const wchar_t *path)
{
    return ((img->list[imgIdx] = lNewImage(hw, path, DBPP)) != NULL);
}

static TFRAME *imageGet (TIMAGES *img, const int imgIdx)
{
    return img->list[imgIdx];
}

static void imageDelete (TIMAGES *img, const int imgIdx)
{
    lDeleteFrame(img->list[imgIdx]);
}

static int loadImageData (THR *hr, TIMAGES *imgs)
{
    wchar_t path[MAX_PATH];
    for (int i = 0; i < 10; i++){
        snwprintf(path, MAX_PATH, L"data/digits/%i.png", i);
        if (!imageAdd(imgs, i, path)){
            wprintf(L"image file missing or corrupt: '%s'\n", path);
            return 0;
        }
    }
    
    // this is allowed to fail so as to allow the user to have no background (white)
    imageAdd(imgs, IMG_BACKGROUND, background);
    return 1;
}

static int antOpen (THR *hr)
{
    int status = libantplus_Open(hr->ant, 0);
    if (status > 0)
        hr->cstates->openCt = 1;
    return status;
}

static void antClose (THR *hr)
{
    if (hr->cstates->openCt)
        libantplus_Close(hr->ant);
    hr->cstates->openCt = 0;
}

static int antGetOpenCt (THR *hr)
{
    return hr->cstates->openCt;
}

static void antCloseChannel (THR *hr)
{
    if (hr->cstates->channelStatus && antGetOpenCt(hr))
        libantplus_CloseChannel(hr->ant, hr->dcfg->channel);
}

THR *new_HR ()
{
    THR *hr = calloc(1, sizeof(THR));
    if (hr){
        hr->dcfg = calloc(1, sizeof(TDCONFIG));
        if (hr->dcfg){
            hr->cstates = calloc(1, sizeof(TCONNSTATES));
            if (hr->cstates){
                hr->rate = calloc(1, sizeof(THRBUFFER));
                if (hr->rate){
                    hr->dev = calloc(1, sizeof(TDEVICET));
                    if (hr->dev){
                        hr->images = calloc(1, sizeof(TIMAGES));
                        if (hr->images){
                            hr->ant = libantplus_Init();
                            if (hr->ant){
                                setRunState(hr, 1);
                                return hr;
                            }
                            free(hr->images);
                        }
                        free(hr->dev);
                    }
                    free(hr->rate);
                }
                free(hr->cstates);
            }
            free(hr->dcfg);
        }
        free(hr);
    }
    
    return NULL;
}

void delete_HR (THR *hr)
{
    if (hr){
        free(hr->ant);
        free(hr->dcfg);
        free(hr->cstates);
        free(hr->dev);
        free(hr->rate);
        free(hr->images);
        free(hr);
    }
}

static void setKey (THR *hr, const int keyIdx)
{
    hr->dcfg->keyIdx = keyIdx;
    hr->cstates->chanIdOnce = 0;
    hr->cstates->channelStatus = 0;
    hr->rate->low = 255;
    hr->rate->high = 0;
    libantplus_ResetSystem(hr->ant);
}

static void drawGraph (THR *hr, const unsigned int colourFil, const unsigned int colourPts)
{
    for (int x = 0; x < frame->width; x++){
        int x1 = frame->width-x-1;
        int y1 = frame->height-1 - hr->rate->bpm[HRBMP_BUFFERLENGTH-x-1];
        lSetPixel(frame, x1, y1, colourPts);
        lDrawLine(frame, x1, y1+1, x1, frame->height-1, colourFil);
    }
}

static void drawPulse (THR *hr, const int bpm)
{
    static unsigned int hr_colour = 0;
    unsigned int x=0,y=0,height=0,width=0;
    if (bpm >= 20 && bpm <= 99){
        TFRAME *d1 = imageGet(hr->images, bpm / 10);
        TFRAME *d2 = imageGet(hr->images, bpm % 10);

        lDrawImage(d1, frame, (frame->width/2 - d1->width) + 3, (frame->height-d1->height)/2);
        lDrawImage(d2, frame, (frame->width/2) - 3, (frame->height - d2->height)/2);
        x = (frame->width/2 - d1->width) + 6;
        y = ((frame->height-d1->height)/2) + 12;
        height = d1->height + 28;
        width = d1->width + d2->width + 50;
    }else if (bpm >= 100 && bpm <= maxhrt){
        TFRAME *d1 = imageGet(hr->images, bpm / 100);
        TFRAME *d2 = imageGet(hr->images, (bpm / 10) % 10);
        TFRAME *d3 = imageGet(hr->images, bpm % 10);

        lDrawImage(d1, frame, (((frame->width - d2->width)/2) - d1->width) + 6 , (frame->height - d1->height)/2);
        lDrawImage(d2, frame, (frame->width - d2->width)/2 , (frame->height - d2->height)/2);
        lDrawImage(d3, frame, (((frame->width + d2->width)/2)) - 6, (frame->height - d3->height)/2);
        x = (((frame->width - d2->width)/2) - d1->width) + 6;
        y = (frame->height - d1->height)/2 + 12;
        height = d1->height+28;
        width = d1->width + d2->width + d3->width;
    }else{
        // You are over your max HRT, call 911! :-)
        TFRAME *d1 = imageGet(hr->images, 9);
        TFRAME *d2 = imageGet(hr->images, 1);
        TFRAME *d3 = imageGet(hr->images, 1);

        lDrawImage(d1, frame, (((frame->width - d2->width)/2) - d1->width) + 6 , (frame->height - d1->height)/2);
        lDrawImage(d2, frame, (frame->width - d2->width)/2 , (frame->height - d2->height)/2);
        lDrawImage(d3, frame, (((frame->width + d2->width)/2)) - 6, (frame->height - d3->height)/2);
        x = (((frame->width - d2->width)/2) - d1->width) + 6;
        y = (frame->height - d1->height)/2 + 12;
        height = d1->height + 28;
        width = d1->width + d2->width + d3->width;
    }

    if (hiit_on)
    {
        if ((elapsed_time < warmup))
        {
            // Warm-up
            if (hiit_on != HIIT_WARMUP)
            {
                // Only do this once
                hr_colour = COLOUR_FATBURNING;
                hiit_on = HIIT_WARMUP;
                curr_interval = intervals+1;
                sprintf(hiit_zone, "%s", HIIT_WARMUP_TAG);
                printf("HIIT Warm-Up, , intervals: %d, et: %I64u\n", intervals, elapsed_time);
            }
            hiit_next_eta = warmup - elapsed_time;
        } else if (elapsed_time >= (warmup + allout*intervals + rest*(intervals) + cooldown))
        {
            // Done
            if (hiit_on!= HIIT_DONE)
            {
                // Only do this once
                hr_colour = COLOUR_WARMUP;
                hiit_on=HIIT_DONE;
                sprintf(hiit_zone, "%s", HIIT_DONE_TAG);
                printf("HIIT Done, intervals: %d, et: %I64u\n", curr_interval, elapsed_time);
            }
            hiit_next_eta = 0;
        } else if (elapsed_time >= (warmup + allout*intervals + rest*intervals))
        {
            // Cool Down
            if (hiit_on != HIIT_COOLDOWN)
            {
                // Only do this once
                hr_colour = COLOUR_FATBURNING;
                hiit_on = HIIT_COOLDOWN;
                curr_interval--;
                sprintf(hiit_zone, "%s", HIIT_COOLIT_TAG);
                printf("HIIT Cool Down, intervals: %d, et: %I64u\n", curr_interval, elapsed_time);
            }
            hiit_next_eta = (warmup + (allout*(intervals-curr_interval)) + (rest*(intervals-curr_interval)) + cooldown) - elapsed_time;
        } else if ((((elapsed_time - warmup) % (rest + allout)) == 0))
        {
            // Giv'er - only hits this on transition
            if (hiit_on != HIIT_GIVER)
            {
                // Only do this once
                hr_colour = COLOUR_VO2MAX;
                hiit_on = HIIT_GIVER;
                curr_interval--;
                sprintf(hiit_zone, "%s", HIIT_MAX_TAG);
                printf("HIIT Giv'er, intervals: %d, et: %I64u\n", curr_interval, elapsed_time);
            }
        } else if ((((elapsed_time - warmup - allout) % (rest + allout)) == 0))
        {
            // Rest - only hits this on transition
            if (hiit_on != HIIT_REST)
            {
                // Only do this once
                hr_colour = COLOUR_FATBURNING;
                hiit_on = HIIT_REST;
                sprintf(hiit_zone, "%s", HIIT_REST_TAG);
                printf("HIIT Rest, intervals: %d, et: %I64u\n", curr_interval, elapsed_time);
            }            
        }

        // These must update every pass also.
        if (hiit_on == HIIT_GIVER)
        {
            hiit_next_eta = (warmup + (allout*(intervals-curr_interval+1)+rest*(intervals-curr_interval)) - elapsed_time);
        } else if (hiit_on == HIIT_REST)
        {
            hiit_next_eta = (warmup + (allout + rest)*(intervals-curr_interval+1)) - elapsed_time;
        }
        lDrawRectangleFilled (frame, x, y, width, height, 160<<24 | hr_colour);
    }
}

static void drawBackground (THR *hr)
{
    TFRAME *background = imageGet(hr->images, IMG_BACKGROUND);
    if (background)
        lDrawImage(background, frame, 0, 0);
    else
        lClearFrame(frame);
}

static int getAve (unsigned char *bpm, const int len)
{
    int ct = 0;
    int sum = 0;
    
    for (int i = (HRBMP_BUFFERLENGTH-len-1); i < HRBMP_BUFFERLENGTH; i++){
        if (bpm[i]){
            sum += bpm[i];
            ct++;
        }
    }
    
    if (ct)
        return sum/(float)ct;
    else
        return 0;
}

static int getMode (unsigned char *bpm, const int len)
{
    unsigned char hist[HRBMP_BUFFERLENGTH];
    memset(hist, 0, HRBMP_BUFFERLENGTH);
        
    for (int i = (HRBMP_BUFFERLENGTH-len-1); i < HRBMP_BUFFERLENGTH; i++)
        hist[bpm[i]]++;
    
    int mode = 0;
    int most = 1;
    for (int i = 20; i < HRBMP_BUFFERLENGTH && i <= 240; i++){
        if (hist[i] > most){
            most = hist[i];
            mode = i;
        }
    }
    
    return mode;
}

//Function to calculate the calories used since the last HRT reading and track the total.
//Male: ((-55.0969 + (0.6309 x HR) + (0.1988 x W) + (0.2017 x A))/4.184) x 60 x T
//Female: ((-20.4022 + (0.4472 x HR) - (0.1263 x W) + (0.074 x A))/4.184) x 60 x T
//HR = Heart rate (in beats/minute) - using average HR for this so it could go up and down as workout changes.
//W = Weight (in kilograms)
//A = Age (in years)
//T = Exercise duration time (in hours)
static double getCals (int hrt, double current_time_ms)
{
    // Find the time since the last heart beat calorie calculation
    double used_calories = 0;

    if ( first_reading )
    {
        // For the first reading since we should be getting 4Hz frequency readings use a delta T of 0.25s or 250ms
        // converted to hours.

        et_ms = 250;
        et_hours = et_ms/MS_IN_HOUR;
        first_reading = 0;
    } else
    {
        // Get the time in ms and convert it to hours
        et_ms = current_time_ms - last_time_ms;
        et_hours = et_ms/MS_IN_HOUR;      // msec to hours
    }
    last_time_ms = current_time_ms;
    if ( male )
    {
        used_calories = ((-55.0969 + (0.6309 * hrt) + (0.1988 * (weight/2.204622)) + (0.2017 * age)) / 4.184) * 60 * et_hours;
    } else
    {
        used_calories += ((-20.4022 + (0.4472 * hrt) - (0.1263 * (weight/2.204622)) + (0.074 * age)) / 4.184) * 60 * et_hours;
    }
    if ( used_calories > 0 )
    {
        calories += used_calories;
    }

    return calories;
}

// Function that returns the Heart Rate Zone the client is currently in based on current HRT
// and wikipedia functions defining the Max HRT and zones.  See http://en.wikipedia.org/wiki/Heart_rate
char* getZone(int hrt)
{
    if ( hrt >= maxhrt-1 )
    {
        danger_ticks++;
        return DANGER_TAG;
    } else if ( hrt >= vo2_rate )
    {
        vo2_ticks++;
        return VO2MAX_TAG;
    } else if ( hrt >= anaerobic_rate )
    {
        anaerobic_ticks++;
        return ANAEROBIC_TAG;
    } else if ( hrt >= aerobic_rate-1 )
    {
        aerobic_ticks++;
        return AEROBIC_TAG;
    } else if ( hrt >= fat_burn_rate-1 )
    {
        fat_burn_ticks++;
        return FATBURNING_TAG;
    } else if ( hrt >= warm_up_rate )
    {
        warm_up_ticks++;
        return WARMUP_TAG;
    } else
    {
        idle_ticks++;
        return REST_TAG;
    }
}

// Function that calculates the clients maximum HRT based on function from 
// wikipedia: HRmax = 191.5 - (0.007 × age^2)  See: http://en.wikipedia.org/wiki/Heart_rate#Formula
static void setMaxHrt ()
{
    maxhrt = (191.5 - (0.007 * age*age));
}

// Function that sets the various zones based on the client's calculated maximum HRT.
// See http://en.wikipedia.org/wiki/Heart_rate#Formula
static void setHrtZones()
{
    vo2_rate = maxhrt * 0.9;
    anaerobic_rate = maxhrt * 0.8;
    aerobic_rate = maxhrt * 0.7;
    fat_burn_rate = maxhrt * 0.6;
    warm_up_rate = maxhrt * 0.5;
}

// Function that returns a string representing the length of the workout in hours, minutes
//  and seconds.  One of the items displayed in the program window.
void getDuration(double current_time_ms)
{
    unsigned int et_rounded_down_hours, et_rounded_down_mins;
    et_hours = (current_time_ms - start_time_ms) / MS_IN_HOUR;
    et_rounded_down_hours = (int)et_hours;
    et_mins = (current_time_ms - start_time_ms - ((int)et_hours * MS_IN_HOUR)) / MS_IN_MINUTE;
    et_rounded_down_mins = (int)et_mins;
    et_secs = (current_time_ms - start_time_ms - ((int)et_hours * MS_IN_HOUR) - ((int)et_mins * MS_IN_MINUTE))/ MS_IN_SECOND;
    sprintf(workout_duration, "%0d:%02d:%02.0f", et_rounded_down_hours, et_rounded_down_mins, et_secs);
}

void drawHeading (THR *hr, const THRBUFFER *rate)
{
    unsigned int hr_colour = 0, scale_width = 0;
    unsigned int scale;
    // Get an accurate current time for the calories burned since last reading calulation.
    double current_time_ms = timeGetTime();
    char zone_str[MAX_ZONE_STR_LENGTH];
    TFRAME *min, *ave, *max;

    sprintf(zone_str, "%s", getZone( (int)(rate->currentBpm) ));

    if ( first_reading )
    {
        // Start the calorie tracking timer
        // Note: Don't clear first_reading here because getCals (called below) needs it too and will clear it.
        last_time = time(NULL);
        start_time = last_time;
        csv_fp=fopen(data_path,"a");
        fprintf(csv_fp, "start_time=%I64u\n", start_time);
        fprintf(csv_fp, CSV_FILE_HEADER);
        fclose(csv_fp);
        start_time_ms = current_time_ms;
    }
    getDuration(current_time_ms);
    // The time in seconds since the workout started.
    elapsed_time = time(NULL) - start_time;

    // Get the calories burned so far this workout
    double calories_so_far = getCals((int)(rate->currentBpm),current_time_ms);
    lSetBackgroundColour(hw, 0x00FFFFFF);
    lSetForegroundColour(hw, 0xFF000000);
    lSetRenderEffect(hw, LTR_OUTLINE2);
    lSetFilterAttribute(hw, LTR_OUTLINE2, 0, 220<<24 | 0xFFFFFF);
    const int font = LFTW_B14;
    total_beats += rate->currentBpm;
    num_readings++; 

    // Row Top 1
    if (hiit_on) {
        min = lNewString(hw, DBPP, 0, font, " HIIT: %s", hiit_zone);
        ave = lNewString(hw, DBPP, 0, font, "Reps: %d\tNext ETA: %I64u", hiit_on==HIIT_WARMUP?intervals:curr_interval, hiit_next_eta);
        max = lNewString(hw, DBPP, 0, font, "Max: %i ", rate->high);
    } else
    {
        min = lNewString(hw, DBPP, 0, font, " Min: %i", rate->low);
        ave = lNewString(hw, DBPP, 0, font, "Win Ave: %i\t\tWin Mode: %i",
                         getAve((unsigned char*)rate->bpm, frame->width),
                         getMode((unsigned char*)rate->bpm, frame->width));
        max = lNewString(hw, DBPP, 0, font, "Max: %i ", rate->high);

    }
    // Row Top 2
    TFRAME *total_ave = lNewString(hw, DBPP, 0, font, " Avg HR: %I64u", total_beats/num_readings);
    TFRAME *maxrate = lNewString(hw, DBPP, 0, font, "MAX HRT: %.0f (%.0f%%)",  maxhrt, (danger_ticks/num_readings)*100);
    TFRAME *workout_time = lNewString(hw, DBPP, 0, font, "Time: %s ", workout_duration);

    // Row Top 3
    TFRAME *zone = lNewString(hw, DBPP, 0, font, " Zone: %s", zone_str);
    TFRAME *cals = lNewString(hw, DBPP, 0, font, "Cals: %.0f\t", calories_so_far);
    TFRAME *burn_rate = lNewString(hw, DBPP, 0, font, "Cals/hr: %.0f ", (calories_so_far/(float)elapsed_time)*SECONDS_IN_HOUR);

    // Row Bottom 1
    TFRAME *namef = lNewString(hw, DBPP, 0, font, " Name: %ls", name);
    TFRAME *agef = lNewString(hw, DBPP, 0, font, "Age: %d", age);
    TFRAME *weightf = lNewString(hw, DBPP, 0, font, "Weight: %d ", weight);

    // Row Bottom 2
    TFRAME *bottom_row2 = lNewString(hw, DBPP, 0, font, " Rest < %.0f (%.0f%%)  Warmup >= %0.f (%.0f%%)  Burn >= %0.f (%.0f%%) ", warm_up_rate, (idle_ticks/num_readings)*100, warm_up_rate, (warm_up_ticks/num_readings)*100, fat_burn_rate, (fat_burn_ticks/num_readings)*100);

    // Row Bottom 3
    TFRAME *bottom_row3 = lNewString(hw, DBPP, 0, font, " O2 >= %0.f (%.0f%%)  No 02 >= %0.f (%.0f%%)  VO2 Max >= %0.f (%.0f%%) ", aerobic_rate, (aerobic_ticks/num_readings)*100, anaerobic_rate,(anaerobic_ticks/num_readings)*100, vo2_rate, (vo2_ticks/num_readings)*100);

    lSetRenderEffect(hw, LTR_DEFAULT);

    lDrawImage(min, frame, 0, ROW_1_TOP_OFFSET);
    lDrawImage(ave, frame, (frame->width - ave->width)/2 , ROW_1_TOP_OFFSET);
    lDrawImage(max, frame, frame->width - max->width, ROW_1_TOP_OFFSET);

    lDrawImage(total_ave, frame, 0, ROW_2_TOP_OFFSET);
    lDrawImage(maxrate, frame, (frame->width - maxrate->width)/2 , ROW_2_TOP_OFFSET);
    lDrawImage(workout_time, frame, frame->width - workout_time->width, ROW_2_TOP_OFFSET);

    /*
     *    Colourize HR zone to show how we are doing
     */
    if ( (int)(rate->currentBpm) >= maxhrt-1 )
    {
        hr_colour = COLOUR_DANGER;
    } else if ( (int)(rate->currentBpm) >= vo2_rate )
    {
        hr_colour = COLOUR_VO2MAX;
    } else if ( (int)(rate->currentBpm) >= anaerobic_rate )
    {
        hr_colour = COLOUR_ANAEROBIC;
    } else if ( (int)(rate->currentBpm) >= aerobic_rate-1 )
    {
        hr_colour = COLOUR_AEROBIC;
    } else if ( (int)(rate->currentBpm) >= fat_burn_rate-1 )
    {
        hr_colour = COLOUR_FATBURNING;
    } else if ( (int)(rate->currentBpm) >= warm_up_rate )
    {
        hr_colour = COLOUR_WARMUP;
    } else
    {
        hr_colour = COLOUR_REST;
    }
    
    lDrawRectangleFilled (frame, ZONE_HIGHLIGHT_OFFSET, ROW_3_TOP_OFFSET+ROW_OFFSET, zone->width, ROW_3_TOP_OFFSET+ZONE_HIGHLIGHT_OFFSET, 160<<24 | hr_colour);

    lDrawImage(zone, frame, 0, ROW_3_TOP_OFFSET);    
    lDrawImage(cals, frame, (frame->width - cals->width)/2 , ROW_3_TOP_OFFSET);
    lDrawImage(burn_rate, frame, frame->width - burn_rate->width, ROW_3_TOP_OFFSET);

    lDrawImage(namef, frame, 0, ROW_1_BOTTOM_OFFSET);
    lDrawImage(agef, frame, (frame->width - agef->width)/2 , ROW_1_BOTTOM_OFFSET);
    lDrawImage(weightf, frame, frame->width - weightf->width, ROW_1_BOTTOM_OFFSET);

    lDrawImage(bottom_row2, frame, 0, ROW_2_BOTTOM_OFFSET);
    lDrawImage(bottom_row3, frame, 0, ROW_3_BOTTOM_OFFSET);

    lDeleteFrame(min);
    lDeleteFrame(ave);
    lDeleteFrame(max);

    lDeleteFrame(total_ave);
    lDeleteFrame(maxrate);
    lDeleteFrame(workout_time);

    lDeleteFrame(zone);
    lDeleteFrame(cals);
    lDeleteFrame(burn_rate);

    lDeleteFrame(namef);
    lDeleteFrame(agef);
    lDeleteFrame(weightf);

    lDeleteFrame(bottom_row2);

    lDeleteFrame(bottom_row3);

    if ( export_to_csv )
    {
        csv_fp=fopen(data_path,"a");
        fprintf(csv_fp, "%I64u,%.3lf,%d,%.0f,%I64u,%i,%i\n",
                num_readings,
                (double)((current_time_ms - start_time_ms)/1000),
                (int)(rate->currentBpm),
                calories,
                total_beats/num_readings,
                rate->high,
                rate->low);
        fclose(csv_fp);
    }
    for (scale = 1;scale<=10;scale++)
    {
        if ( (SCALE_OFFSET*scale)%40 != 0)
        {
            scale_width = SCALE_WIDTH;
        } else
        {
            scale_width = SCALE_WIDTH*2;
        }
        lDrawLine (frame, DWIDTH-scale_width, DHEIGHT - (scale*SCALE_OFFSET), DWIDTH, DHEIGHT - (scale*SCALE_OFFSET), 160<<24 | COLOUR_RED);
        lDrawLine (frame, 0, DHEIGHT - (scale*SCALE_OFFSET), scale_width, DHEIGHT - (scale*SCALE_OFFSET), 160<<24 | COLOUR_RED);         
    }
}

static void doWindowLoop (THR *hr)
{
    if ( hr->cstates->vDisplay )
    {
        MSG message;
        while ( PeekMessage(&message, NULL, 0, 0, PM_REMOVE) )
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
}

// search for a hrm by trying each of the available keys in sequence
int doHRMSearch (THR *hr, const int attempts, const int timeout)
{

    int ct, readOK;
    int retries = attempts;
            
    do{
        int keyIdx = 0;
        do{         
            setKey(hr, keyIdx);
            ct = timeout;
                                    
            do{
                do{
                    drawBackground(hr);
                    marqueeDraw(hr, frame, hr->marquee);
                    //drawHeading(hr, hr->rate);
                    lRefresh(frame);
                    lSleep(100);

                    readOK = libantplus_HandleMessages(hr->ant);
                    if (readOK == LIBUSB_DISCONNECT) return -1;
                    
                    if (kb_hit()) setRunState(hr, 0);                   
                }while(readOK > 0 && !hr->dev->scidDeviceNumber && getRunState(hr));
            }while(ct-- && !hr->dev->scidDeviceNumber && getRunState(hr));
        }while(keyIdx++ < KEY_TOTAL && !hr->dev->scidDeviceNumber && getRunState(hr));
    }while(retries-- && !hr->dev->scidDeviceNumber && getRunState(hr));

    int found = hr->dev->scidDeviceNumber && getRunState(hr);
    if (found)
        dbprintf(hr, "Found HRM %i", hr->dev->scidDeviceNumber);
    return found;
}

// block until the ant+ usb dongle is found
int waitForDeviceConnect (THR *hr)
{
    int found = 0;
    
    do{
        doWindowLoop(hr);
        drawBackground(hr);
        dbprintf(hr, "searching for Ant+ stick (0x%.4X/0x%.4X)", ANTSTICK_VID, ANTSTICK_PID);
        
        found = libantplus_Discover(hr->ant, 0);
        if (found){
            dbprintf(hr, "Ant+ USB stick found");
            if (hr->ant->strings[0][0])
                dbprintf(hr, " %s ",hr->ant->strings[0]);
            if (hr->ant->strings[1][0] && hr->ant->strings[2][0])
                dbprintf(hr, " %s - %s ",hr->ant->strings[1], hr->ant->strings[2]);
            else if (hr->ant->strings[1][0])
                dbprintf(hr, " %s ",hr->ant->strings[1]);       
        }
        
        marqueeDraw(hr, frame, hr->marquee);            
        lRefresh(frame);
        
        if (kb_hit()) setRunState(hr, 0);
        if (!found && getRunState(hr))
            lSleep(900);
    }while(!found && getRunState(hr));

    return found;
}

#if G19CB

DWORD softkeyg19cb (int device, DWORD dwButtons, struct TMYLCDG19 *mylcdg19)
{
    TMYLCDG19 *g19 = (TMYLCDG19*)mylcdg19;
    if (g19){
        THR *hr = (THR*)g19->ptrUser;
        if (hr){
            if (LGLCDBUTTON_OK == dwButtons)
                setRunState(hr, 0);
        }
    }
    return 1;
}

static void assignG19Callback (THR *hr, void *softkeycb)
{

    lDISPLAY did = lDriverNameToID(hw, "G19", LDRV_DISPLAY);
    if (did){
        TMYLCDG19 *lcdg19;
        lGetDisplayOption(hw, did, lOPT_G19_STRUCT, (intptr_t*)&lcdg19);
        lcdg19->ptrUser = hr;
        lSetDisplayOption(hw, did, lOPT_G19_SOFTKEYCB, (intptr_t*)softkeycb);
    }else{
        hr->cstates->vDisplay = 1;
    }

}
#endif

void printhelp()
{
    fprintf(stderr ,"\nUsage: Ant_HRMxx.exe [-h|?] "
          "<configfilename> (see settings.ini for example of config file"
          "\n\t-h | -? : Print this help"
          "\n\t-f      : config file name");
}

int main (int argc, char *argv[])
{
    FILE *settings_fp;
    DIR *dirPtr;
    struct tm * broketime;
    static time_t    caltime;
    int index;

    if (argc == 2)
    {
        if (((argv[1][0] == '/') || (argv[1][0] == '-')) && ((argv[1][1] == 'h') || (argv[1][1] == '?')))
        {
            printhelp();
            return(0);
        }
    } else
    {
        printf("Incorrect usage: Wrong number of args [%d]\n", argc);
        printhelp();
        return (0);
    }

    if((settings_fp=fopen(argv[1], "r")) == NULL)
    {
        printf("Incorrect usage: Cannot open config file %s.\n", argv[1]);
        printhelp();
        return(1);
    } else
    {
        fwscanf(settings_fp, L"Name: %s\nAge: %u\nWeight (lbs): %u\nHeight (feet): %u\nHeight (inches): %f\nSex (M/F): %s\nHIIT (0-No, 1-yes): %u\nIntervals: %u\nWarmUp (s): %u\nAllOut (s): %u\nRest (s): %u\nCoolDown (s): %u\nBackground (png file 320x240): %s\nExport (0-No, 1-yes): %u",
                 name, &age, &weight, &height_f, &height_i, sex, &hiit_on, &intervals, &warmup, &allout, &rest, &cooldown, background, &export_to_csv);
        fclose(settings_fp);

        if (strncmp(sex,MALE,1) == 0)
        {
            male = 1;
        } else
        {
            male = 0;
        }
        setMaxHrt();
        setHrtZones();
        printf("\n%s settings read from %s:\nName: %ls\nAge: %u\nWeight: %u lbs\nHeight: %u ft %0.1f in\nSex: %s\nMale: %d\nMax Heart Rate: %.0f\nV02 Max: %.0f\nAnaerobic Rate: %.0f\nAerobic Rate: %.0f\nFat Burn Rate: %.0f\nWarm Up Rate: %.0f\nHIIT On: %s\nIntervals: %u\nWarm Up: %u s\nAll Out: %u s\nRest: %u s\nCool Down: %u s\nBackground image: %ls\n\n", 
               argv[0], argv[1], name, age, weight, height_f, height_i, sex, male, maxhrt, vo2_rate, anaerobic_rate, aerobic_rate, fat_burn_rate, warm_up_rate, hiit_on?"Yes":"No", intervals, warmup, allout, rest, cooldown, background);
    }

    if (!initMYLCD_USBD480())
        return EXIT_FAILURE;

    THR *hr = new_HR();
    if (hr == NULL){
        printf("startup failed allocating memory\n");
        return EXIT_FAILURE;
    }

    if (!loadImageData(hr, hr->images)){
        delete_HR(hr);
        return EXIT_FAILURE;
    }

    hr->marquee = marqueeNew(16, MARQUEE_LEFT);
#if G19CB
    assignG19Callback(hr, softkeyg19cb);
#else
    hr->cstates->vDisplay = 1;
#endif
    drawBackground(hr);
    lRefresh(frame);
        
    if (hr->ant){
        hr->dcfg->deviceNumber = 0;     // 0 
        hr->dcfg->deviceType = 0x78;    // 1
        hr->dcfg->transType = 0;        // 5
        hr->dcfg->channelType = 0;      
        hr->dcfg->networkNumber = 0;
        hr->dcfg->channel = 0;
        hr->dcfg->channelPeriod = /*4096*/8070;
        hr->dcfg->RFFreq = /*0x32*/0x39;
        hr->dcfg->searchTimeout = 255;
        hr->dcfg->searchWaveform = 0x53;
        libantplus_SetEventFunction(hr->ant, EVENTI_MESSAGE, messageEventCb, hr);
                                
        unsigned char graph[DWIDTH];
        memset(graph, 0, sizeof(graph));
        int dfound = 0;     // is stick found and connected
        int hfound = 0;     // has hrm been found and synced
        int readOK = 1;

        // Setup the csv data file
        if ( export_to_csv )
        {
            time(&caltime);
            broketime = localtime(&caltime);
            strftime(workout_csv_filename2,MAX_WORKOUT_CSV_FILENAME_LENGTH,WORKOUT_CSV_FILENAME,broketime);
            memcpy((void*)workout_csv_filename,(void*)name, (int)wcslen(name));
            for (index=0;index <= (int)wcslen(name);index++)
            {
                memcpy((void*)(workout_csv_filename+index),(void*)(name+index), 1);
            }
            // Use the name as the data folder
            strcat(data_path, "./");
            strcat(data_path, workout_csv_filename);
            strcat(data_path, "/");
            // See if the folder already exists
            if((dirPtr = opendir(data_path)) == NULL)
            {
                printf("Couldn't open directory %s, creating\n", data_path);
                if (mkdir(data_path) == 0)
                {
                    printf("Created directory %s\n", data_path);
                } else
                {
                    printf("Failed creating directory %s\n", data_path);
                }
            } else
            {
                printf("Directory %s already exists\n", data_path);
                closedir(dirPtr);
            }
            strcat(workout_csv_filename, workout_csv_filename2);
            printf("Creating workout data file location from: [%s] and [%s] \n\n", data_path, workout_csv_filename);
            strcat(data_path,workout_csv_filename);
            printf("Creating workout data file: %s\n\n", data_path);

            csv_fp=fopen(data_path,"a");
            if (csv_fp == NULL)
            {
                printf("Can't create file: %s \n", data_path);
                return 1;
            } else
            {
                fprintf(csv_fp, "Name=%ls\n", name);
                fprintf(csv_fp, "Max HRT=%.0f\n", maxhrt);
                fprintf(csv_fp, "V02=%.0f\n", vo2_rate);
                fprintf(csv_fp, "Anaerobic=%.0f\n", anaerobic_rate);
                fprintf(csv_fp, "Aerobic=%.0f\n", aerobic_rate);
                fprintf(csv_fp, "Fat Burn=%.0f\n", fat_burn_rate);
                fprintf(csv_fp, "Warm Up=%.0f\n", warm_up_rate);
                fclose(csv_fp);
            }
        }

        do{
            do{
                doWindowLoop(hr);
                        
                if (!dfound){
                    dfound = waitForDeviceConnect(hr);
                    if (dfound && getRunState(hr)){
                        antClose(hr);
                        dfound = antOpen(hr);
                    }
                    hfound = 0;
                }

                if (!hfound && getRunState(hr)){
                    hfound = doHRMSearch(hr, CONNECTION_RETRIES, CONNECTION_TIMEOUT);
                    if (hfound == -1) dfound = 0;

                }else if (getRunState(hr)){
                    drawBackground(hr);
                    if (!marqueeDraw(hr, frame, hr->marquee))
                        drawHeading(hr, hr->rate);
                    drawGraph(hr, 80<<24 | 0xF01010, 160<<24 | 0xFF0000);
                    drawPulse(hr, hr->rate->currentBpm);
                    lRefresh(frame);
                    readOK = libantplus_HandleMessages(hr->ant);
                }
                if (kb_hit()) setRunState(hr, 0);
            }while(readOK > 0 && getRunState(hr));

            if (readOK == LIBUSB_DISCONNECT && getRunState(hr)){
                libantplus_ResetSystem(hr->ant);
                antClose(hr);
                hr->dev->scidDeviceNumber = 0;
                hfound = 0;
                dfound = 0;
                readOK = 1;
                
            // if we're here then contact between HRM and skin could be poor
            // or HRM may be poorly fitted
            }else if (readOK < 0){
                dbprintf(hr, "read error, retrying...");
                lSleep(100);
            }
        }while(getRunState(hr));
    
        antCloseChannel(hr);
        antClose(hr);
    }

    for (int i = 0; i < IMG_TOTAL; i++)
        imageDelete(hr->images, i);
    closeMYLCD_USBD480();
    marqueeDelete(hr->marquee);
    delete_HR(hr);
    
    return EXIT_SUCCESS;
}


