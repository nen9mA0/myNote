DAY = 1
YEAR = 365 * DAY

def kernel_mktime(tm_year,tm_mon,tm_mday,tm_hour,tm_min,tm_sec):
    year = tm_year-70
    res = year * YEAR + DAY * ((year+1)/4)
    if 
