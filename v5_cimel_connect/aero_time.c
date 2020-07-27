#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "aero_time.h"

//

//  
//  /*  FUNCTION:  get_GMT																	*
//   *	Description:  This function will get the moment in time since 1970.					*	
//   *  Include Library:  servnew1.h, time.h													*
//   *  Local Variables:  NONE																*
//   *  Global Variables: mom -- pointer structure that includes date and time information	*
//   *  Function Calls:  increase_time														*
//   *  AUTHOR:  Ilya Slutsker																*
//   *  Change History:  Ilya Slutsker (Original)											*
//   *					 Dave Giles - (6/20/2001) - Added documentation						*/
//  
//  void get_GMT(mom)
//  TIME_POINT *mom;
//  
void get_GMT(TIME_POINT *mom)
{

mom->year = 70; /*assign 70 for 1970*/
mom->month = 1; /*assign 1 for January*/
mom->day = 1; /*assign 1 for the 1st day of the month*/
mom->hour = mom->minute = mom->second = 0;  /*assign time to be exactly midnight GMT*/

increase_time(mom,1. * time(NULL) /86400.);  /*record moment in time*/
}
//

//  
//  
//  //-1-1-1 
//  /*  FUNCTION:  get_julian																			*
//   *	Description:  This function determines the julian day (including fraction)						* 
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  i -- counters
//   *					  k -- number of days since beginning of year				
//   *	Global Variables: mom -- pointer to the mom structure
//   *  Function Calls:  NONE																			*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/20/2001) - Added documentation									*/
//  
//  double get_julian(mom)
//  TIME_POINT *mom;
//  
double get_julian(TIME_POINT *mom)
{
int  i,k;
k = 0;/* tracks number of days since beginning of year */

if (mom->month > 1) {/* find number of months and add total number of days of each month */
	for (i = 1; i < mom->month; i++)/* once current month is reached, break and determine current day */
		k += get_monday(i,mom->year);
}
k += mom->day-1;/* current day minus 1 to get fraction of the day in hours/minutes/seconds */
return k + (mom->hour * 3600. + mom->minute * 60. + mom->second)
/ 86400.;/* return total julian day fraction */
}

//  
//  
//  //-1-1-1 
//  /*  FUNCTION:  get_monday																			*
//   *	Description:  This function returns the total possible days in a specific month					* 
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  NONE
//   *	Global Variables: i -- integer representing month
//   *					  year -- integer representing the year
//   *  Function Calls:  NONE													*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/21/2001) - Added documentation									*/
//  
//  int get_monday(i,year)
//  int i, year;
//  
int get_monday(int i,int year)
{
	switch (i) {
/* for months that have 30 days */
	case 4:
	case 6:
	case 9:
	case 11:
		return 30;

/* for February 28 or 29 days */
	case 2 :
		if (year %4 ) return 28;
		else return 29;

/* otherwise, for all other months that have 31 days */
	default :
 		return 31;
	}
}

//  
//  //-1-1-1 
//  /*  FUNCTION:  get_now																	*
//   *	Description:  This program will get the moment in time since 1970 in local time.	*	
//   *  Include Library:  servnew1.h, time.h													*
//   *  Local Variables:  ptr_time --  pointer to time structure tm
//   *					  nseconds -- integer in nanoseconds
//   *  Global Variables: mom -- pointer structure that includes date and time information	*
//   *  Function Calls:  NONE																*
//   *  AUTHOR:  Ilya Slutsker																*
//   *  Change History:  Ilya Slutsker (Original)											*
//   *					 Dave Giles - (6/20/2001) - Added documentation						*/
//  
//  void get_now(mom)
//  TIME_POINT *mom;
//  
void get_now(TIME_POINT *mom)
{
struct tm *ptr_time;
long nseconds;

nseconds = time(NULL);  /*number of seconds since 1970*/
ptr_time = localtime(&nseconds);  /*call time.h function for local time*/
mom->year = ptr_time->tm_year;  /*current year - 1900*/
mom->month = ptr_time->tm_mon + 1; /*month 0 to 11 (need to add 1 for correction)*/
mom->day = ptr_time->tm_mday ; /*day of the month*/
mom->minute = ptr_time->tm_min;  /*minute after the hour*/
mom->second = ptr_time->tm_sec; /*seconds after the minute*/
mom->hour = ptr_time->tm_hour; /*hours since midnight*/
}

//  
//  
//  //-1-1-1 
//  /*  FUNCTION:  increase_time																		*
//   *	Description:  This function determines current time since 1970 or a past time					* 
//   *  Include Library:  servnew1.h, math.h															*
//   *  Local Variables:  mom -- pointer to moment time structure
//   *					  dellla -- type double fraction of days
//   *	Global Variables: delka -- type double temporary fraction of days
//   *					  
//   *  Function Calls:  get_monday																		*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/21/2001) - Added documentation
//  								- deleted restka (not used in function)								*/
//  
//  void increase_time(mom, dellla)
//  TIME_POINT *mom;
//  double dellla;
//  
void increase_time(TIME_POINT *mom,double dellla)
{
double delka = dellla;  

if (dellla > 0.)  {

	/*determine fraction of the day*/
	delka  = dellla + 1./24. * mom->hour  + 1./1440. * mom->minute + 1./86400 * mom->second;

/* keep track of current day/month/year by looping through time since 1970 */
while (delka > 1.) {
	mom->day++;/* increment day */
/* check to make sure current day value does not exceed total possible in month */
	if (mom->day > get_monday(mom->month,mom->year)) {
		mom->day = 1;/* if exceeds, assign first day of next month */
		mom->month ++;/* increment month */
/* check to make sure months to not exceed total possible in year */
		if (mom->month > 12) {
			mom->month = 1;/* if exceeds, assign first month of next year */
			mom->year++;/* increment year */
		}
	}
delka -= 1.;/* decrement total days since 1970 */
}

delka *= 24;/* find hours from day fraction */
mom->hour = floor(delka);
delka -= mom->hour;

/* find minutes from day fraction */
delka *= 60;
mom->minute = floor(delka);
delka -= mom->minute;

/* find seconds from day fraction */
mom->second = floor(delka * 60);
}
/* else ...determine the date backward in time */
else
 {
/* fraction of time in past */
delka  = dellla + 1./24. * mom->hour  + 1./1440. * mom->minute + 1./86400 * mom->second;

	while (delka <0. ) {
		mom->day--;/* decrement day */
		if (mom->day < 1 ) {
			mom->month --;/* if day is zero, decrement month */

			if (mom->month <1) {
				mom->month = 12;
				mom->year--;/* if month is zero, decrement year */
			}

/* if day = 0, then get total days in previous month*/
		mom->day = get_monday(mom->month,mom->year);
		}
	delka += 1.;/* increment day fraction */
	}

/* find hour from day fraction */
delka *= 24;
mom->hour = floor(delka);
delka -= mom->hour;

/* find minutes from day fraction */
delka *= 60;
mom->minute = floor(delka);
delka -= mom->minute;

/* find seconds from day fraction */
mom->second = floor(delka * 60);
}


if (mom->hour == 24)
{
mom->hour = 0;
increase_time_day(mom,1);

}


}

//  
//  //-1-1-1 
//  /*  FUNCTION:  increase_time_day																	*
//   *	Description:  This function increases the day a certain amount (depending on delta passed)		*
//   *										  														*   
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  rest -- keep track of days
//   *					   
//   *	Global Variables: mom -- pointer to moment time structures
//   *					  dela -- delta of time to increase (or decrease, if negative)
//   *  Function Calls:	  increase_time_month, get_monday																			*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/22/2001) - Added documentation	
//   *								- deleted (int) day, year, month, ikil (not used in function)				*/
//  
//  void increase_time_day(mom,dela)
//  TIME_POINT *mom;
//  int dela;
//  
void increase_time_day(TIME_POINT *mom,int dela)
{
int rest,ikil;

if (dela == 0) return;/* no increase in day necessary */

else if (dela > 0) {/* if delta is greater than 0, increase day */
/* the number of possible days minus current day plus 1 to increase day */
	rest = get_monday(mom->month,mom->year)- mom->day + 1;
	while (rest < dela){/* adjust current day and month if necessary */
		dela -= rest;/* decrement della from rest */
		mom->day = 1;/* assign first day of the month */
		increase_time_month(mom,1);/* increase the month */
		rest = get_monday(mom->month,mom->year);/* get new total days in month and break */
	}
	mom->day += dela;/* increase day */
	if (mom->day > (ikil = get_monday(mom->month,mom->year))) {
		mom->day -= ikil;/* decrease day by the number of days in the month to get first day */
		increase_time_month(mom,1);/* increase month */
	}
}
else{/* if dela is less than zero */
	rest = mom->day;/* current moment day is assign to rest */
	while (rest  <= -dela){/* if the current day -dela (where dela is negative)*/
		dela += rest;/* increase day (but decrease in magnitude) */
		increase_time_month(mom,-1);/* decrease month (-1 parameter) */
		mom->day = rest = get_monday(mom->month,mom->year);/* get total number of days in month */
	}
	mom->day += dela;/* increase day (but decrease in magnitude) */
}
}

//  
//  
//  
//  //-1-1-1 
//  /*  FUNCTION:  time_copy																			*
//   *	Description:  This function copies elements of one time structure to another					* 
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  NONE
//   *	Global Variables: mom1,mom2 -- pointers to moment time structures
//   *  Function Calls:  NONE													*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/21/2001) - Added documentation									*/
//  
//  void time_copy(mom1,mom2)
//  TIME_POINT *mom1, *mom2;
//  
//  
void time_copy(TIME_POINT *mom1,TIME_POINT *mom2)
{
	mom1->year = mom2->year;
	mom1->month = mom2->month;
	mom1->day = mom2->day;
	mom1->hour = mom2->hour;
	mom1->minute = mom2->minute;
	mom1->second = mom2->second;
}

//  
//  
//  
//  //-1-1-1 
//  /*  FUNCTION:  time_day_difference																	*
//   *	Description:  This function finds day difference between two moments (adjusts month if necessary)*
//   *										  														*   
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  minn,maxx -- pointer to moment time structures
//   *					  scin -- value of moment structure
//   *					  ikk, i, ikil -- counter and temporary variables
//   *	Global Variables: mom1,mom2 -- pointers to moment time structures
//   *  Function Calls:	  time_copy, time_month_difference, increase_time_month, get_monday																			*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/22/2001) - Added documentation									*/
//  
//  int time_day_difference(mom1,mom2)
//  TIME_POINT *mom1, *mom2;
//  
int time_day_difference(TIME_POINT *mom1,TIME_POINT *mom2)
{
TIME_POINT *minn, *maxx, scin;
int ikk, i, ikil;
/* determine which year is most recent */
if (mom1->year < mom2->year) ikk = 1;
else if (mom1->year > mom2->year) ikk = -1;
/* if years are equal, then determine most recent month */
else if (mom1->month < mom2->month) ikk = 1;
else if (mom1->month > mom2->month ) ikk = -1;
/* otherwise, return the difference in days */
else
return mom2->day - mom1->day;

/* assign most recent moment as max and oldest as min */
if (ikk == 1) {minn = mom1;
maxx = mom2;}
else {minn = mom2;
maxx = mom1;
}
/* find day difference */
ikil = maxx->day - minn->day;
/* copy minn into scin */
time_copy(&scin, minn);
/* determine difference in month between moments */
while ( time_month_difference(&scin, maxx) > 0)
{/* get total days in month for each difference in month*/
ikil += get_monday(scin.month, scin.year);
increase_time_month(&scin,1);/* increase month by one */
}
return ikil * ikk; /* return number of days (negative if mom2 is older than mom1)*/
}

//  
//  
//  //-1-1-1 
//  /*  FUNCTION:  time_difference																		*
//   *	Description:  This function determines the time difference between time points					* 
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  i,j,k -- integer number of years
//   *					  sumec -- double type sum of julian days
//   *					  jday1,jday2 -- double type julian day
//   *					  sign -- negative or positive sign
//   *	Global Variables: mom1, mom2 -- pointers to the mom structure
//   *  Function Calls:  get_julian																		*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/21/2001) - Added documentation									*/
//  
//  double time_difference(mom1,mom2)
//  TIME_POINT *mom1, *mom2;
//  
//  
double time_difference(TIME_POINT *mom1,TIME_POINT *mom2)
{TIME_POINT *mmm1,*mmm2;
int i,j,k, sign;
double sumec,jday1,jday2;

i = mom1->year;
j = mom2->year;

if (i!=j) {
if (i < j ) { mmm1 = mom1; /* if mom2 > mom1 then mom2 is most recent - diff is positive */
mmm2 = mom2; sign = 1;}
else {mmm2= mom1; /* otherswise, mom2 < mom1 and diff is negative */
k = i; i = j; j = k; /* reassign i and j */
mmm1 = mom2; sign = -1;}
/* get julian days for each moment */
jday1 = get_julian(mmm1);
jday2 = get_julian(mmm2);

if (mmm1->year % 4) sumec= 365.-jday1; /* when not leap year */
else sumec = 366.-jday1; /* otherwise, if leap year */

if (j-i > 1){ /* if greater than one year difference -- add more days for each year */
	for (k = i+1; k< j;k++) {
		if (k%4) sumec += 365.;/* when not leap year */
		else sumec += 366.; /* if leap year */
	}
}
return (sumec + jday2) * sign; /* return difference in number of days and sign */
}
else {jday1 = get_julian(mom1); /* if the same year, then get julian days */
jday2 = get_julian(mom2);
return jday2-jday1;} /* subtract julian days to get difference */
}


//  
//  //-1-1-1 
//  /*  FUNCTION:  increase_time_month																	*
//   *	Description:  This function increases the month value to the next month 
//   *				  (advances year if necessary)														*   
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  NONE
//   *	Global Variables: mom -- pointers to moment time structures
//   *					  dela -- delta to increase month
//   *  Function Calls:  NONE												*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/22/2001) - Added documentation									*/
//  
//  void increase_time_month(mom,dela)
//  TIME_POINT *mom;
//  int dela;
//  
void increase_time_month(TIME_POINT *mom,int dela)
{
mom->month += dela;/* change month by delta (+ or -) */
while (mom->month > 12)/* if month exceeds 12 */
{
mom->year ++;/* increment year */
mom->month -= 12;/* reset to first month */
}
while (mom->month < 1)/* if month decreases below first month */
{
mom->year --;/* decrement year */
mom->month += 12;/* assign to last month */
}

}

//  
//  //-1-1-1 
//  /*  FUNCTION:  time_month_difference																*
//   *	Description:  This function finds the month difference between two moments					* 
//   *  Include Library:  servnew1.h																		*
//   *  Local Variables:  NONE
//   *	Global Variables: mom1,mom2 -- pointers to moment time structures
//   *  Function Calls:  NONE													*
//   *  AUTHOR:  Ilya Slutsker																			*
//   *  Change History:  Ilya Slutsker (Original)														*
//   *					 Dave Giles - (6/22/2001) - Added documentation									*/
//  
//  int time_month_difference(mom1, mom2)
//  TIME_POINT *mom1, *mom2;
//  
int time_month_difference(TIME_POINT *mom1,TIME_POINT *mom2)
{
return 12 * (mom2->year - mom1->year) + mom2->month - mom1->month;
}
