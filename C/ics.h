#ifndef _ICS_H_
#define _ICS_H_

#define DT_LEN       17
#define SUMMARY_LEN  80
#define LOCATION_LEN 80
#define RRULE_LEN    80

typedef struct event_t
{
    char dtstart[DT_LEN];
    char dtend[DT_LEN];
    char summary[SUMMARY_LEN];
    char location[LOCATION_LEN];
    char rrule[RRULE_LEN];
} event_t;

#endif
