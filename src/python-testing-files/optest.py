#dp algorithm

# seg1 = [[2, 9],
#         [1, 10],
#         [0, 10.5],
#         [-1, 11],
#         [-2, 12]]

# seg2 = [[2, 5.5],
#         [1, 5.8],
#         [0, 6],
#         [-1, 6.5],
#         [-2, 6.8]]

# seg3 = [[3, 5],
#         [2, 5.5],
#         [1, 5.8],
#         [0, 6],
#         [-1, 6.5],
#         [-2, 6.8],
#         [-3, 6.8]]

seg1 = [[1, 10.5],
        [0, 10]]

seg2 = [[-2, 5.5],
        [-1, 6.5],
        [0, 8.5],
        [1, 9.0]]

seg3 = [[0, 5],
        [1, 5]]

starting_battery = 0
max = 3

track = [seg1, seg2, seg3]


# 5 for number of deployment/harvesting options that each segment can have
# 3 for number of segments on the track
# 3 for the number of different states that the ending battery can be in
table = [[[-1 for i in range(5)] for i in range(3)] for i in range(3)]
choice = [[[-1 for i in range(5)] for i in range(3)] for i in range(3)]

# The main DP algorithm
# Limitation, only runs when battery is in range : 0 <= battery <= 4, and ending_battery is in range 0 <= ending_battery <= 4
def optimization(index, battery, ending_battery):
    global table
    global track
    best_path = None

    if index == len(track):
        return 0
    elif table[ending_battery][index][battery] != -1:
        return table[ending_battery][index][battery]

    best_time = float('inf')
    total_time = float('inf')
    current_seg = track[index]

    for option in current_seg:
        if index != len(track) - 1:
            if battery + option[0] >= 0 and battery + option[0] <= max:
                remaining_time = optimization(index + 1, battery + option[0], ending_battery)
                total_time = remaining_time + option[1]
            
        else:
            if battery + option[0] >= ending_battery and battery + option[0] <= max:
                remaining_time = optimization(index + 1, battery + option[0], ending_battery)
                total_time = remaining_time + option[1]

        if total_time < best_time:
            best_time = total_time
            best_path = option

    table[ending_battery][index][battery] = best_time
    choice[ending_battery][index][battery] = best_path

    return best_time

# Reconstruct the path
# Limitation, only runs when battery is in range : 0 <= battery <= 4
# only runs after optimization is done, ie run on the same starting battery level as the table
def path_construction(battery, ending_battery):
    global choice
    global track
    path = []

    local_table = choice[ending_battery]

    for i in range(len(local_table)):
        path.append(local_table[i][battery])
        battery += local_table[i][battery][0]

    return path

ending_battery = 2
starting_battery = 1

print(optimization(0, starting_battery, ending_battery))
print(table)
print(path_construction(starting_battery, ending_battery))
print(choice)






    