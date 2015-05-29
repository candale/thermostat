temp_hist = [23.199, 23.299, 23.199, 23.199, 23.199, 23.199, 23.299,
             23.199, 23.199, 23.199]
temp_hist_r = [l for l in reversed(temp_hist)]
avg = -10000
records_per_minute = 2
avg = temp_hist[1] - temp_hist[0]
for i in range(1, 9):
    avg = (avg + temp_hist[i + 1] - temp_hist[i]) / 2

print avg
