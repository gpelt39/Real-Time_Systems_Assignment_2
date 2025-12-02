import sys
import csv
import json

# Default input and output file paths
input_file = 'data.csv'
output_file = 'output.json'

# Function to convert CSV data to the required JSON format with preemptions handled
# The example format that is needed: 
# {
#     "name": "task4",
#     "cat": "task",
#     "ph": "B",
#     "pid": 4,
#     "tid": "task4",
#     "ts": 0
# }

def process_data(input_file, output_file):

    # List to store logging data after adding preemptions
    after_preempt_data = []

    # Stack to track the currently running tasks
    active_tasks = []

    # Open the CSV file for reading
    with open(input_file, mode='r') as csvfile:
        csvreader = csv.reader(csvfile)

        # Add lines to the logging to model preemption
        for row in csvreader:
            task_n = int(row[0])  # Extract task number as integer
            phase = 'B' if row[1] == '1' else 'E'  # Convert phase
            timestamp = int(row[2])  # Convert timestamp to integer

            # If the logging mentions that a task begins ...
            if phase == 'B':
                # ... we check if there's already a running task.
                if active_tasks != []:
                    # If so, we need to add a line to preempt it.
                    running_task = active_tasks[-1]
                    after_preempt_data.append((running_task, 'E', timestamp))                        
                
                # In any case, we add the new task.
                active_tasks.append(task_n)
                after_preempt_data.append((task_n, phase, timestamp))
            
            # If the logging mentions that a task ends ...
            elif phase == 'E':
                running_task = active_tasks[-1]
                # ... we check for sanity that it is actually the currently active task ...
                if task_n == running_task:
                    # ..and then add it to the list and remove it from the active tasks.
                    after_preempt_data.append((task_n, phase, timestamp))
                    active_tasks.pop()
                else:
                    print("ERROR: current ending task " + str(task_n) + " is not the last active task " + str(running_task) + " (at timestamp" + str(timestamp) + ")")
                    return
                
                # If there is an active task left (due to preemption) ...
                if active_tasks != []:
                    # ... we should restart it.
                    ready_task = active_tasks[-1]
                    after_preempt_data.append((ready_task, 'B', timestamp))
                    
    
    # It may be the case that there are some tasks that start and then end at the same timestamp due to preemption shenanigans.
    # We want to filter these out because they look weird on the tracing tool.
    after_zero_filter_data = [];
    last_starting_task = ()
    
    for line in after_preempt_data:
        (task_n, phase, timestamp) = line
        if phase == 'B':
            last_starting_task = line;
            after_zero_filter_data.append(line);
        else:
            if last_starting_task == (task_n, 'B', timestamp):
                after_zero_filter_data.pop()
            else:
                after_zero_filter_data.append(line);
    
    # We also filter out the last line if it is a task begin line so that it does not end with an empty task.
    if after_zero_filter_data[-1][1] == 'B':
        after_zero_filter_data.pop()
    
    # Lastly, we convert every line to a json output for the chrome://tracing tool
    json_data = []
    for task_n, phase, timestamp in after_zero_filter_data:
        json_data.append({
            "name": f"task{task_n}",
            "cat": "task",
            "ph": phase,
            "pid": task_n,
            "tid": f"task{task_n}",
            "ts": timestamp * 1000
        })

    # Write the JSON data to the output file
    with open(output_file, mode='w') as jsonfile:
        json.dump(json_data, jsonfile, indent=4)

# process parameters
if len(sys.argv) > 1:
        input_file = sys.argv[1]
        if len(sys.argv) > 2:
            output_file = sys.argv[2]

# Run the process
process_data(input_file, output_file)

print(f"Data processed and written to {output_file}")
