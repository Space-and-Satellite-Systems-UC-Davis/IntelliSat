# Creates histogram showing the number of times per day the satellite was in range across 3 weeks

import matplotlib.pyplot as plt


FILE_NAME = 'INPUT' # Ex: 'inrange30.txt'
DEGREES = INPUT # Ex: 30


def plot_in_range(filename, degrees):
    data = []
    minute = 1
    day = 1
    prev_value = None

    with open(filename) as f:
        for line in f:
            if line.strip():
                # Count the number of times satellite is in range (0 --> 1 transitions)
                value = int(line)
                if value == 1 and prev_value == 0:
                    data.append(day)
                # Iterate the day value based on number of minutes (1440 minutes in 1 day)
                if minute == 1440:
                    minute = 1
                    day += 1
                else:
                    minute += 1

                prev_value = value

    bins = [i for i in range(1, 22)]
    plt.hist(data, bins=bins, edgecolor='black')

    plt.title(f'Number of Times in Range (Elevation Angle {degrees}°)')
    plt.xlabel('Day')
    plt.ylabel('Number of Times in Range')
    plt.xticks(range(1, 22))

plot_in_range(FILE_NAME, DEGREES)
plt.show()