
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"
#include "ics.h"
#include "listy.h"
#include <time.h>

#define MAX_LINE_LEN 132

void dt_format(char *formatted_time, const char *dt_time, const int len);
void dt_increment(char *after, const char *before, int const num_days);
void e_period(char *begin, char *end, char *time);
int find_repeat_end(char *repeat_line);

/*
 * Function time_in_interval.
 * 
 * return 1 if the begin <= time <= end,
 * otherwise, return 0
 */
int time_in_interval(char *time, int begin, int end){
    char *time_date = (char *)calloc(MAX_LINE_LEN, sizeof(char));
    strncpy(time_date, time, 8); //year,month,day are totally 8 characters
    int date = atoi(time_date);
    free(time_date);
    return (date >= begin && date <= end);
}

/*
 * Function combine_date.
 * 
 * conbine the 4 digits number year, 2 digits number month and day, to a signle 8 digit number
 * then, return the 8 digints yearmonthday number.
 */
int combine_date(int year, int month, int day){
    char *date = (char *)calloc(MAX_LINE_LEN, sizeof(char));
    sprintf(date,"%d%02d%02d",year,month,day);
    int date_i = atoi(date);
    free(date);
    return date_i;
}

/*
 * Function find_repeat_end.
 *
 * Find the end time string in the repeat_line
 * then return the end_time integer.
 */
int find_repeat_end(char *repeat_line){
    char *copy_line2;
    int line_len = strlen(repeat_line); line_len++; // add '\0'
    char *copy_line = (char *)calloc(line_len, sizeof(char));
    char *end_d = (char *)calloc(line_len, sizeof(char));
    
    strncpy(copy_line, repeat_line, line_len);
    copy_line2 = strtok(copy_line,";");
    while (copy_line2 != NULL){
        if (strlen(copy_line2) == 21){ // "UNTIL=...T..." is 21 char.
            strncpy(end_d,copy_line2+6, 8);    // "UNTIL=" is 6 char.
            break;
        }
        copy_line2 = strtok(NULL,";");
    }
    int result = atoi(end_d);
    free(copy_line); free(end_d);
    return result;
}

/*
 * Function argv_proper.
 * 
 * check if the argument have correct formate
 * if not, exit with failure.
 */
void argv_proper(int from_y, int to_y, char **filename, char *argv[]){
    if (from_y == 0 || to_y == 0 || *filename == NULL) {
        fprintf(stderr, 
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }
}

/*
 * Function get_arguments.
 * 
 * Get the filename string, and from_date and to_date int.
 * call argv_prope to exit if arguments is not poper formated.
 */
void get_arguments(int argc, char *argv[], char **filename, int* from_date, int* to_date){
    int i;
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    
    for (i = 0; i < argc; i++) {
        if (!strncmp(argv[i], "--start=", 8)) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (!strncmp(argv[i], "--end=", 6)) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (!strncmp(argv[i], "--file=", 7)) {
            *filename = argv[i]+7;
        }
    }
    argv_proper(from_y, to_y, filename, argv);
    *from_date = combine_date(from_y, from_m, from_d);
    *to_date = combine_date(to_y, to_m, to_d);
}

/*
 * Function add_mulit_events
 * 
 * make several nodes contain repeat events,     
 * add these node to list inorder according to event_t's start time.
 */
void add_mulit_events(node_t **list, event_t per_event, int from_date, int to_date){
    if (!strncmp(per_event.rrule,"", RRULE_LEN) 
        && time_in_interval(per_event.dtstart, from_date, to_date)){
        *list = add_inorder(*list, new_node(per_event));     
    }else{
        char *begin = calloc(DT_LEN, sizeof(char)), *end = calloc(DT_LEN, sizeof(char));
        char *summary = calloc(SUMMARY_LEN, sizeof(char)); char *location = calloc(LOCATION_LEN, sizeof(char));
        char *rrule= calloc(RRULE_LEN, sizeof(char));
        strncpy(begin, per_event.dtstart, DT_LEN); strncpy(end, per_event.dtend,  DT_LEN);
        strncpy(summary, per_event.summary, SUMMARY_LEN); strncpy(location, per_event.location, LOCATION_LEN);
        strncpy(rrule, per_event.rrule, RRULE_LEN);
        
        int repeat_begin = atoi(begin);
        int repeat_end = find_repeat_end(per_event.rrule);
        while (repeat_begin < repeat_end){
            if (time_in_interval(per_event.dtstart,from_date, to_date)){
                strncpy(per_event.summary, summary, SUMMARY_LEN);
                strncpy(per_event.location, location, LOCATION_LEN); 
                strncpy(per_event.rrule, rrule, RRULE_LEN);
                *list = add_inorder(*list, new_node(per_event));
            }
            dt_increment(per_event.dtstart, per_event.dtstart, 7); // one week is 7 days
            dt_increment(per_event.dtend, per_event.dtend, 7);
            strncpy(begin, per_event.dtstart, 8); // //year,month,day are totally 8 characters
            repeat_begin = atoi(begin);
        }
        free(begin); free(end); free(summary); free(location); free(rrule);
    }
}
/*
 * Function read_file
 * 
 * read every line inside the file, and construct the event_t
 * then pick those event_t inside the time range, add them inorder to list
 */
void read_file(FILE *file, node_t **list, int from_date, int to_date){
    char *line = NULL, *sub;
    event_t per_event = {"", "", "", "", ""};
    size_t len = 0;
    while((getline(&line, &len, file)) != -1){
        sub = strtok(line,":");
		if (strcmp(sub, "DTSTART") == 0) {
            sub = strtok(NULL,"\n"); 
			strncpy(per_event.dtstart, sub, DT_LEN);
        }else if(strcmp(sub, "DTEND") == 0) {
			sub = strtok(NULL,"\n");
			strncpy(per_event.dtend, sub, DT_LEN);
		}else if(strcmp(sub, "LOCATION") == 0) {
			sub = strtok(NULL,"\n");
			if (sub == NULL)
				strncpy(per_event.location, "", LOCATION_LEN);
			else
				strncpy(per_event.location, sub, LOCATION_LEN);
        }else if(strcmp(sub, "SUMMARY") == 0) {
            sub = strtok(NULL,"\n");
            strncpy(per_event.summary, sub, SUMMARY_LEN);
        }else if (strcmp(sub, "RRULE") == 0){
            sub = strtok(NULL,"\n");
            strncpy(per_event.rrule, sub, RRULE_LEN);
        }else if (strcmp(sub, "END") == 0){
            sub = strtok(NULL,"\n");
            if (strcmp(sub, "VEVENT") == 0){
                add_mulit_events(list, per_event, from_date, to_date);
                strncpy(per_event.rrule,"", RRULE_LEN);
            }
        }
    }
    if (line) free(line);
}

/*
 * Function print_date:
 * 
 * print the date string and the line"---" in proper formate.
 * 
 */
void print_date(char *date){
    char *date_str =(char *)malloc(MAX_LINE_LEN * sizeof(char));
    dt_format(date_str, date, MAX_LINE_LEN);
    int length = strlen(date_str);
    char *split =(char *)calloc(MAX_LINE_LEN, sizeof(char));
    for (int i = 0; i<length; i++)
		strcat(split,"-");
    printf("%s\n%s\n",date_str,split);
    free(date_str); free(split);
}

/*
 * Function print_events:
 * 
 * Print formatted event_t in the node_t. 
 */
void print_events(node_t *event, void *arg){
    char *prev_date = (char *)arg;
    char *cur =(char *)malloc(9 * sizeof(char)); //year,month,day are totally 8 characters + '\0'
    char *period = (char *)malloc(MAX_LINE_LEN * sizeof(char)); 

    strncpy(cur, event->val.dtstart,8);
    if (strncmp(cur,prev_date,8)){
        print_date(event->val.dtstart);
        strncpy(prev_date,cur,8);
    }
    e_period(event->val.dtstart, event->val.dtend, period);
    printf("%s: %s {{%s}}\n",period, event->val.summary,event->val.location);

    if (event->next != NULL){
        char next[9]; //year,month,day are totally 8 characters + '\0'
        strncpy(next, event->next->val.dtstart, 8); 
        if (strncmp(cur,next,8))
            printf("\n");
    }
    free(period); free(cur);
}

/*
 * Function analysis:
 * 
 * Print formatted all event_t in the list.
 */
 void analysis(node_t *list) {
    char *prev_date = (char *)calloc(DT_LEN, sizeof(char));
    apply(list, print_events, prev_date);  
    free(prev_date);
 }

/*
 * Function main:
 * 
 * Crate a list to store all event_t, which is in the argument's range,
 * then print these event_t in proper format.
 */
int main(int argc, char *argv[])
{
    int from_date, to_date;
    char *filename = NULL;
    
    get_arguments(argc, argv, &filename, &from_date, &to_date);  // test if argument is correct; if it's correct, get infomation from it
    
    node_t *list = NULL;    
	FILE *file = fopen(filename,"r");
    read_file(file, &list, from_date, to_date);
    analysis(list);

    node_t  *temp_n;
    for ( ; list != NULL; list = temp_n ) {
        temp_n = list->next;
        free(list);
    }

	fclose(file);
    exit(0);
}


/*
 * Function dt_format.
 *
 * Given a date-time, creates a more readable version of the
 * calendar date by using some C-library routines. For example,
 * if the string in "dt_time" corresponds to:
 *
 *   20190520T111500
 *
 * then the string stored at "formatted_time" is:
 *
 *   May 20, 2019 (Mon).
 *
 */
void dt_format(char *formatted_time, const char *dt_time, const int len)
{
    struct tm temp_time;
    time_t    full_time;

    /*  
     * Ignore for now everything other than the year, month and date.
     * For conversion to work, months must be numbered from 0, and the 
     * year from 1900.
     */  
    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d",
        &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_time, len, "%B %d, %Y (%a)", 
        localtime(&full_time));
}

/*
 * Function dt_increment:
 *
 * Given a date-time, it adds the number of days in a way that
 * results in the correct year, month, and day. For example,
 * if the string in "before" corresponds to:
 *
 *   20190520T111500
 *
 * then the datetime string stored in "after", assuming that
 * "num_days" is 100, will be:
 *
 *   20190828T111500
 *
 * which is 100 days after May 20, 2019 (i.e., August 28, 2019).
 *
 */
void dt_increment(char *after, const char *before, int const num_days)
{
    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2dT%2d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday, &temp_time.tm_hour, 
        &temp_time.tm_min, &temp_time.tm_sec);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 16, "%Y%m%dT%H%M%S", localtime(&full_time));
    after[MAX_LINE_LEN - 1] = '\0';
}

/*
 * Function e_period:
 *
 * Make the *begin and *end time in a proper format, 
 * then store it into the time point to.
 */
void e_period(char *begin, char *end, char *time){
    int begin_h, begin_m, end_h, end_m;
	char *begin_hour = calloc(3, sizeof(char)), *begin_min = calloc(3, sizeof(char)), 
    *end_hour = calloc(3, sizeof(char)), *end_min = calloc(3, sizeof(char));

	strncpy(begin_hour, begin+9, 2); // begin_hour && end_hour is from 9's position
    strncpy(begin_min, begin+11, 2); // bigin_min && end_min is from 11's position 
    strncpy(end_hour, end+9, 2); strncpy(end_min, end+11, 2);
	begin_h = atoi(begin_hour); begin_m = atoi(begin_min); end_h = atoi(end_hour); end_m = atoi(end_min);
	free(begin_hour); free(begin_min); free(end_hour); free(end_min);

	//time_translate
	if (begin_h >= 12){
		if (begin_h != 12){
			begin_h -= 12;
			if (begin_h == 12){
				if (end_h < 12)
					sprintf(time,"%*d:%02d AM to %*d:%02d AM",2,begin_h,begin_m,2,end_h,end_m);
				else if(end_h > 12){
					end_h -= 12;
					sprintf(time,"%*d:%02d AM to %*d:%02d PM",2,begin_h,begin_m,2,end_h,end_m);
				}else
					sprintf(time,"%*d:%02d AM to %*d:%02d PM",2,begin_h,begin_m,2,end_h,end_m);
			}else{
				end_h -= 12; 
				sprintf(time,"%*d:%02d PM to %*d:%02d PM",2,begin_h,begin_m,2,end_h,end_m);
			}
		}else{
			if (end_h != 12)
				end_h -= 12;
			sprintf(time,"%*d:%02d PM to %*d:%02d PM",2,begin_h,begin_m,2,end_h,end_m);
		}
	}else{
		if (end_h < 12)
			sprintf(time,"%*d:%02d AM to %*d:%02d AM",2,begin_h,begin_m,2,end_h,end_m);
		else if (end_h ==12)
			sprintf(time,"%*d:%02d AM to %*d:%02d PM",2,begin_h,begin_m,2,end_h,end_m);
		else{
			end_h -= 12; 
			sprintf(time,"%*d:%02d AM to %*d:%02d PM",2,begin_h,begin_m,2,end_h,end_m);
		}
	}

}	