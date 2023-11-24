import os
from os.path import exists
from flask import Flask, render_template, request, send_file
from database_tools import *
from datetime import *
#app = Flask(__name__, template_folder='/home/pi/Desktop/coap/COAP_SERVER/templates')
app = Flask(__name__, template_folder='')

# function to evaluate if the threshold configuration submitted by the user is valid
def is_user_threshold_input_ok(threshold_list):
    if len(threshold_list) == 4:
        # Temp threshold
        if threshold_list[0] > 80 or threshold_list[0] < 0 or threshold_list[0] == None:
            return False
        # hum threshold
        if threshold_list[1] > 100 or threshold_list[1] < 0 or threshold_list[1] == None:
            return False
        # soil threshold
        if threshold_list[2] > 100 or threshold_list[2] < 0 or threshold_list[2] == None:
            return False
        # light threshold
        if threshold_list[3] > 100 or threshold_list[3] < 0 or threshold_list[3] == None:
            return True
        return True
    else:
        return False

# function that handles a submit of a configuration for thresholds (soil moisture, humidity ... etc)
def submit_threshold_configuration(id):
    thresholds = []
    # get the values user entered
    temp_form = 'row_{id}_temperature'.format(id=str(id))
    hum_form = 'row_{id}_humidity'.format(id=str(id))
    soil_form = 'row_{id}_soil_moisture'.format(id=str(id))
    light_form = 'row_{id}_light'.format(id=str(id))
    # Check what user entered for temperature, humidity, soil moisture and light exposure as thresholds
    val = request.form[temp_form]
    if val != '':
        thresholds.append(int(val))
    val = request.form[hum_form]
    if val != '':
        thresholds.append(int(val))
    val = request.form[soil_form]
    if val != '':
        thresholds.append(int(val))
    # val = request.form[light_form]
    # random value TODO: cleanup the light threshold field
    val = 300
    if val != '':
        thresholds.append(int(val))

    # if user entered a valid threshold configuration, accept it and handle it
    if is_user_threshold_input_ok(thresholds):
        UserRequestsDBTools().store_threshold_config_request(id, thresholds[0], thresholds[1], thresholds[2], thresholds[3])
        return True
    # else return error code
    else:
        return False

# function that handles a submit of a configuration for thresholds (soil moisture, humidity ... etc)
def submit_control_configuration(id):
    user_control = []
    # get the values user entered
    light_switch_form = 'Light switch {id}'.format(id=str(id))
    water_switch_form = 'Water switch {id}'.format(id=str(id))
    fan_switch_form = 'Fan switch {id}'.format(id=str(id))
    automatic_control = 'Automatic control switch {id}'.format(id=str(id))
    # Check what user entered for temperature, humidity, soil moisture and light exposure as thresholds
    val = request.form.get(light_switch_form)
    if val != 'None':
        user_control.append(1)
    else:
        user_control.append(0)
    val = request.form.get(water_switch_form)
    if val != 'None':
        user_control.append(1)
    else:
        user_control.append(0)
    val = request.form.get(fan_switch_form)
    if val != 'None':
        user_control.append(1)
    else:
        user_control.append(0)
    val = request.form.get(automatic_control)
    if val != 'None':
        user_control.append(1)
    else:
        user_control.append(0)
    UserRequestsDBTools().store_control_config_request(id, user_control[0], user_control[1], user_control[2], user_control[3])

def clear_measurement_diagrams():
    UserRequestsDBTools().clear_measurements_diagrams()

timestamps=[]
temperatures_row_1=[]
temperatures_row_2=[]
temperatures_row_3=[]
temperatures_row_4=[]
temperatures_row_5=[]
humidity_row_1=[]
humidity_row_2=[]
humidity_row_3=[]
humidity_row_4=[]
humidity_row_5=[]
soil_moisture_row_1=[]
soil_moisture_row_2=[]
soil_moisture_row_3=[]
soil_moisture_row_4=[]
soil_moisture_row_5=[]
light_exposure_row_1=[]
light_exposure_row_2=[]
light_exposure_row_3=[]
light_exposure_row_4=[]
light_exposure_row_5=[]

def create_data(len):
    global timestamps
    global temperatures_row_1
    global temperatures_row_2
    global temperatures_row_3
    global temperatures_row_4
    global temperatures_row_5
    global humidity_row_1
    global humidity_row_2
    global humidity_row_3
    global humidity_row_4
    global humidity_row_5
    global soil_moisture_row_1
    global soil_moisture_row_2
    global soil_moisture_row_3
    global soil_moisture_row_4
    global soil_moisture_row_5
    global light_exposure_row_1
    global light_exposure_row_2
    global light_exposure_row_3
    global light_exposure_row_4
    global light_exposure_row_5

    timestamps   = GetDatabaseEntries().get_most_recent_timestamps(len)
    # Then get the values
    temperatures_row_1 = GetDatabaseEntries().create_list_of_data(1, 'temperature', timestamps)
    temperatures_row_2 = GetDatabaseEntries().create_list_of_data(2, 'temperature', timestamps)
    temperatures_row_3 = GetDatabaseEntries().create_list_of_data(3, 'temperature', timestamps)
    temperatures_row_4 = GetDatabaseEntries().create_list_of_data(4, 'temperature', timestamps)
    temperatures_row_5 = GetDatabaseEntries().create_list_of_data(5, 'temperature', timestamps)
    humidity_row_1 = GetDatabaseEntries().create_list_of_data(1, 'humidity', timestamps)
    humidity_row_2 = GetDatabaseEntries().create_list_of_data(2, 'humidity', timestamps)
    humidity_row_3 = GetDatabaseEntries().create_list_of_data(3, 'humidity', timestamps)
    humidity_row_4 = GetDatabaseEntries().create_list_of_data(4, 'humidity', timestamps)
    humidity_row_5 = GetDatabaseEntries().create_list_of_data(5, 'humidity', timestamps)
    soil_moisture_row_1 = GetDatabaseEntries().create_list_of_data(1, 'soil_moisture', timestamps)
    soil_moisture_row_2 = GetDatabaseEntries().create_list_of_data(2, 'soil_moisture', timestamps)
    soil_moisture_row_3 = GetDatabaseEntries().create_list_of_data(3, 'soil_moisture', timestamps)
    soil_moisture_row_4 = GetDatabaseEntries().create_list_of_data(4, 'soil_moisture', timestamps)
    soil_moisture_row_5 = GetDatabaseEntries().create_list_of_data(5, 'soil_moisture', timestamps)
    light_exposure_row_1 = GetDatabaseEntries().create_list_of_data(1, 'light_exposure', timestamps)
    light_exposure_row_2 = GetDatabaseEntries().create_list_of_data(2, 'light_exposure', timestamps)
    light_exposure_row_3 = GetDatabaseEntries().create_list_of_data(3, 'light_exposure', timestamps)
    light_exposure_row_4 = GetDatabaseEntries().create_list_of_data(4, 'light_exposure', timestamps)
    light_exposure_row_5 = GetDatabaseEntries().create_list_of_data(5, 'light_exposure', timestamps)

@app.route('/')
def create_diagrams_page():
    create_data(60)
    return render_template('index.html', 
    temperature_values_1=temperatures_row_1, 
    temperature_values_2=temperatures_row_2,
    temperature_values_3=temperatures_row_3, 
    temperature_values_4=temperatures_row_4, 
    temperature_values_5=temperatures_row_5, 
    humidity_values_1=humidity_row_1, 
    humidity_values_2=humidity_row_2, 
    humidity_values_3=humidity_row_3, 
    humidity_values_4=humidity_row_4, 
    humidity_values_5=humidity_row_5,
    soil_moisture_values_1=soil_moisture_row_1,
    soil_moisture_values_2=soil_moisture_row_2,
    soil_moisture_values_3=soil_moisture_row_3, 
    soil_moisture_values_4=soil_moisture_row_4, 
    soil_moisture_values_5=soil_moisture_row_5, 
    light_exposure_values_1=light_exposure_row_1,
    light_exposure_values_2=light_exposure_row_2,
    light_exposure_values_3=light_exposure_row_3,
    light_exposure_values_4=light_exposure_row_4,
    light_exposure_values_5=light_exposure_row_5,
    labels=timestamps)

@app.route('/', methods=['POST'])
def my_form_post():
    if request.form['action'] == 'Submit row 1 thresholds':
        submit_id=1
        if submit_threshold_configuration(submit_id):
            return 'Done'
        else:
            return 'Wrong configuration'
    elif request.form['action'] == 'Submit row 2 thresholds':
        submit_id=2
        if submit_threshold_configuration(submit_id):
            return 'Done'
        else:
            return 'Wrong configuration'
    elif request.form['action'] == 'Submit row 3 thresholds':
        submit_id=3
        if submit_threshold_configuration(submit_id):
            return 'Done'
        else:
            return 'Wrong configuration'
    elif request.form['action'] == 'Submit row 4 thresholds':
        submit_id=4
        if submit_threshold_configuration(submit_id):
            return 'Done'
        else:
            return 'Wrong configuration'
    elif request.form['action'] == 'Submit row 5 thresholds':
        submit_id=5
        if submit_threshold_configuration(submit_id):
            return 'Done'
        else:
            return 'Wrong configuration'
    elif request.form['action'] == 'Submit row 1 switches settings':
        submit_id = 1
        submit_control_configuration(submit_id)
        # call function
        return 'Row 1 control configured'
    elif request.form['action'] == 'Submit row 2 switches settings':
        submit_id = 2
        submit_control_configuration(submit_id)
        # call function
        return 'Row 2 control configured'
    elif request.form['action'] == 'Submit row 3 switches settings':
        submit_id = 3
        submit_control_configuration(submit_id)
        # call function
        return 'Row 3 control configured'
    elif request.form['action'] == 'Submit row 4 switches settings':
        submit_id = 4
        submit_control_configuration(submit_id)
        # call function
        return 'Row 4 control configured'
    elif request.form['action'] == 'Submit row 5 switches settings':
        submit_id = 5
        submit_control_configuration(submit_id)
        # call function
        return 'Row 5 control configured'
    elif request.form['action'] == 'Clear diagrams':
        clear_measurement_diagrams()
        return 'Clear diagrams'
    return 'What do you want user???'

# NOTE: The following code is garbage. Hardcoded non-maintainable stuff. Needs to be written in a more structured way -------------------------------------------
@app.route('/generate_row_1_temps')
def generate_row_1_temps():
    create_data(0)
    txt = ''
    file_name = "row_1_temperatures.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if temperatures_row_1[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {temperatures_row_1[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_2_temps')
def generate_row_2_temps():
    create_data(0)
    txt = ''
    file_name = "row_2_temperatures.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if temperatures_row_2[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {temperatures_row_2[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_3_temps')
def generate_row_3_temps():
    create_data(0)
    txt = ''
    file_name = "row_3_temperatures.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if temperatures_row_3[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {temperatures_row_3[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_4_temps')
def generate_row_4_temps():
    create_data(0)
    txt = ''
    file_name = "row_4_temperatures.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if temperatures_row_4[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {temperatures_row_4[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_5_temps')
def generate_row_5_temps():
    create_data(0)
    txt = ''
    file_name = "row_5_temperatures.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if temperatures_row_5[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {temperatures_row_5[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_1_hum')
def generate_row_1_hum():
    create_data(0)
    txt = ''
    file_name = "row_1_humidity.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if humidity_row_1[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {humidity_row_1[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_2_hum')
def generate_row_2_hum():
    create_data(0)
    txt = ''
    file_name = "row_2_humidity.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if humidity_row_2[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {humidity_row_2[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_3_hum')
def generate_row_3_hum():
    create_data(0)
    txt = ''
    file_name = "row_3_humidity.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if humidity_row_3[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {humidity_row_3[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_4_hum')
def generate_row_4_hum():
    create_data(0)
    txt = ''
    file_name = "row_4_humidity.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if humidity_row_4[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {humidity_row_4[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_5_hum')
def generate_row_5_hum():
    create_data(0)
    txt = ''
    file_name = "row_5_humidity.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if humidity_row_5[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {humidity_row_5[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_1_light')
def generate_row_1_light():
    create_data(0)
    txt = ''
    file_name = "row_1_light_exposure.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if light_exposure_row_1[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {light_exposure_row_1[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_2_light')
def generate_row_2_light():
    create_data(0)
    txt = ''
    file_name = "row_2_light_exposure.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if light_exposure_row_2[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {light_exposure_row_2[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_3_light')
def generate_row_3_light():
    create_data(0)
    txt = ''
    file_name = "row_3_light_exposure.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if light_exposure_row_3[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {light_exposure_row_3[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_4_light')
def generate_row_4_light():
    create_data(0)
    txt = ''
    file_name = "row_4_light_exposure.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if light_exposure_row_4[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {light_exposure_row_4[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_5_light')
def generate_row_5_light():
    create_data(0)
    txt = ''
    file_name = "row_5_light_exposure.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if light_exposure_row_5[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {light_exposure_row_5[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_1_soil')
def generate_row_1_soil():
    create_data(0)
    txt = ''
    file_name = "row_1_soil_moisture.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if soil_moisture_row_1[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {soil_moisture_row_1[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_2_soil')
def generate_row_2_soil():
    create_data(0)
    txt = ''
    file_name = "row_2_soil_moisture.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if soil_moisture_row_2[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {soil_moisture_row_2[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_3_soil')
def generate_row_3_soil():
    create_data(0)
    txt = ''
    file_name = "row_3_soil_moisture.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if soil_moisture_row_3[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {soil_moisture_row_3[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_4_soil')
def generate_row_4_soil():
    create_data(0)
    txt = ''
    file_name = "row_4_soil_moisture.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if soil_moisture_row_4[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {soil_moisture_row_4[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

@app.route('/generate_row_5_soil')
def generate_row_5_soil():
    create_data(0)
    txt = ''
    file_name = "row_5_soil_moisture.txt"

    with open(file_name, 'w') as file:
        file.write('timestamp; measurement\n')
        for idx in range(len(timestamps)):
            if soil_moisture_row_5[idx] != 'None':
                line_to_write = f'{timestamps[idx]}; {soil_moisture_row_5[idx]}\n'
                file.write(line_to_write)

    return send_file(file_name, as_attachment=True)

# -------------------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    app.run(host="::", port=1068, debug=True)
