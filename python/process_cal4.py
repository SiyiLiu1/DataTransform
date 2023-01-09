#!/usr/bin/env python3
"""
process_cal4.py

Input a ics file and a date, the process_cal class will convert and 
return a formated String. 

Some code copy from my own assignment2

For example:
February 02, 2021 (Tue)
-----------------------
10:30 AM to 11:30 AM: ECON 104 {{DSB C112}}
11:30 AM to 12:30 PM: ASTR 101 {{ELL 067}}
"""
import re
from datetime import datetime as dt
from datetime import timedelta as td


class Event:
    """
    Input 4 strings: start, end, location, summary
    """
    def __init__(self, start, end, location, summary):
        self.start = start
        self.end = end
        self.location = location
        self.summary = summary
        
    def __repr__(self):
        """
        Input 2 datetime.datetime: start, end
        and 2 strings: location, summary
        
        Output a string with correct format like:
            ==TIME_LINE==: ==SUMMARY== {{==LOCATION==}}
        """
        event_line = "==TIME_LINE==: ==SUMMARY== {{==LOCATION==}}"
        text = re.sub(r"==TIME_LINE==", self.hour_min(), event_line)
        text = re.sub(r"==SUMMARY==", self.summary, text)
        text = re.sub(r"==LOCATION==", self.location, text)
        return text

    def hour_min(self) -> str:
        """
        Input a datetime.datetime
        return a format string like " 2:30 PM"
        """
        start_hour = self.start.strftime("%-I")
        end_hour = self.end.strftime("%-I")
        if len(start_hour) == 1:
            start_hour = " " + str(start_hour)
        else:
            start_hour = str(start_hour)
            
        if len(end_hour) == 1:
            end_hour = " " + str(end_hour)
        else:
            end_hour = str(end_hour)

        return start_hour + ":" + self.start.strftime("%M %p") \
                + " to " + end_hour + ":"  +self.end.strftime("%M %p")
    
    def time_travel(self, repeat_end:dt, day:dt, exdate:dt) -> bool:
        """
        According to the start time and the repeat_end time, find the day
            will have this event or not.
        If the repeat_end == dt(1, 1, 1, 0, 0), means the event doesn't repeat, 
            then find the event will happened on that day or not.
        """
        if repeat_end == dt(1, 1, 1, 0, 0):
            return self.start.date() == day.date()
        else:
            if self.start.date() <= day.date() and day.date() <= repeat_end.date():
                while self.start.date() <= day.date():
                    if self.start.date() == day.date() and self.start.date() != exdate.date():
                        return True
                    self.start += td(7) # one week is 7 days
            return False



class process_cal:
    """
    class process_cal:
    Input a ics file and print those events inside the file according to argv's 
       date and time
    """
    def __init__(self, ics_file):
        """
        instantiate that class process_cal
        ics_file is the input file name
        """
        self.ics_file = ics_file
        
    def get_events_for_day(self, day:dt) -> str:
        """
        Input a datetime.datetime object,
        Return a string in the samew day with the datetime.datetime object,
            string listing events according to the start time.
        """
        file_handle = open(self.ics_file,"r")
        result = self.file_read(file_handle, day)
        file_handle.close()
        return result

    def file_read(self, file_handle, day:dt) -> str:
       """
       read the ics file line by line,
       and return all events format as a string
       """
       pattern = re.compile(r"(.*):(.*)")
       repeat_end = dt(1, 1, 1, 0, 0) # initialize the repeat_end to a impossible value
       exdate = dt(1, 1, 1, 0, 0) # initialize the exdate to a impossible value
       time_dict = {}
       for line in file_handle:
           line = line.strip()
           m = pattern.search(line)
           if m:
               if m.group(1) == "DTSTART":
                   start = self.str_to_time(m.group(2))
               elif m.group(1) == "DTEND":
                   end = self.str_to_time(m.group(2))
               elif m.group(1) == "LOCATION":
                   location = m.group(2)
               elif m.group(1) == "SUMMARY":
                   summary = m.group(2)
               elif m.group(1) == "RRULE":
                   repeat_end = self.find_repeat_end(m.group(2))
               elif m.group(1) == "EXDATE":
                   exdate = self.str_to_time(m.group(2))
               elif m.group(1) == "END":
                   if m.group(2) == "VEVENT":
                       e = Event(start, end, location, summary)
                       if e.time_travel(repeat_end, day, exdate):
                           time_dict[start.time()] = e
                       repeat_end = dt(1, 1, 1, 0, 0) # initialize the repeat_end
                       exdate = dt(1, 1, 1, 0, 0)  # initialize the exdate
       return self.find_events(time_dict, day)
   

    def find_events(self, events:dict, day:dt)-> str:
        """
        Input a Dictionary
        return a string of all events in the day following by 
        chronological order based on the event's starting time.
        """
        if events != {}:
            result = ""
            event_num = len(events)
            count = 0
            key_list = sorted(events.keys())
            result = self.date_line(day) +"\n"
            result += "-" * len(self.date_line(day)) + "\n"
            
            for time in key_list:
                result += str(events[time])
                count += 1
                if count != event_num:
                    result += "\n"
            return result

    def date_line(self, date:dt) -> str:
        """
        Input a datetime.datetime object
        Transfer datetime.datetime object to proper date string format
        """
        return date.strftime("%B %d, %Y (%a)")

    def find_repeat_end(self, info: str)-> dt:
        """
        Find the end of the repeat date from the input str, and return it
        """
        repeat_list = info.split(";")
        for each in repeat_list:
            find_end = False 
            if each[:5] == "UNTIL" and not find_end:
                repeat_end = self.str_to_time(each[6:])
                find_end = True
        return repeat_end                        
                            
            
    def str_to_time(self, time:str) -> dt:
        """
        Deal with string with format like "20210214T180000",
            translate them to a datetime.datetime
        """
        return dt.strptime(time,"%Y%m%dT%H%M%S")


