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

seg1 = [[1, 10],
        [0, 10]]

seg2 = [[-2, 5.5],
        [-1, 6.5],
        [0, 8.5],
        [1, 9.0]]

seg3 = [[0, 5],
        [1, 5]]

starting_battery = 0
max = 3
time = 0
route = []


choice = [[-1 for i in range(5)] for i in range(3)]
track = [seg1, seg2, seg3]

table = [[-1 for i in range(5)] for i in range(3)]
def optimization(index, battery):
    global table
    global track
    best_path = None

    if index == len(track):
        return 0
    elif table[index][battery] != -1:
        return table[index][battery]

    best_time = float('inf')
    current_seg = track[index]

    for option in current_seg:
        if battery + option[0] >= 0 and battery + option[0] <= max:
            remaining_time = optimization(index + 1, battery + option[0])
            total_time = remaining_time + option[1]
        
            if total_time < best_time:
                best_time = total_time
                best_path = option

    table[index][battery] = best_time
    choice[index][battery] = best_path

    return best_time

def path_construction(battery):
    global choice
    global track
    path = []

    for i in range(len(choice)):
        path.append(choice[i][battery])
        battery += choice[i][battery][0]

    return path


print(optimization(0, starting_battery))
print(path_construction(starting_battery))
print(table)






    