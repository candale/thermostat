from prettytable import PrettyTable
import math
def to_fr(temp):
	return temp * 1.8 + 32

def to_c(temp):
	return (temp - 32) / 1.8

def get_temp_index(temp, humidity):
	hi = (-42.379 + 2.04901523*temp + 10.14333127*humidity - 
	 	   0.22475541*temp*humidity - 0.00683783*temp*temp - 
	 	   0.05481717*humidity*humidity + 0.00122874*temp*temp*humidity + 
	 	   0.00085282*temp*humidity*humidity - 0.00000199*temp*temp*humidity*humidity)

	if humidity < 13 and temp >= 80 and temp <= 112:
		adj = ((13-humidity) / 4.0) * math.sqrt((17-abs(temp-95)) / 17.0)
		hi = hi - adj 

	if humidity > 85 and temp >= 80 and temp <= 87:
		adj = ((humidity-85)/10.0) * ((87-temp)/5.0)
		hi = hi + adj

	return hi

result = []
for humidity in range(20, 80, 10):
	res = []
	for temp in range(16, 26):
		tf = to_fr(temp)
		res.append(round(to_c(get_temp_index(tf, humidity)), 2))
	res.insert(0, "{}%".format(humidity))
	result.append(res)

header = range(16, 26)
header.insert(0, 'humidity/temp')
table = PrettyTable(header)
for res in result:
	table.add_row(res)

print table



