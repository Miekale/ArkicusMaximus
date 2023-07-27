#pragma config(Motor,  motorA,           ,             tmotorEV3_Large, openLoop, encoder)
#pragma config(Motor,  motorD,           ,             tmotorEV3_Large, openLoop, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "PC_FileIO.c"

// DEBUG OUTPUT
TFileHandle fout;

// Global Hardware Constants
float const GEAR_RADIUS_X = 11.93; // mm
float const GEAR_RADIUS_Y = 11.93;
float const GEAR_RADIUS_Z = 13.081;
float const PEN_DISTANCE = 1;

// Global Software Constants (UI and File information)
const int MAX_FILES = 4;
const int MAX_SHAPES = 2;

const string fileArray[MAX_FILES] = {"Among Us", "NASSAR GOAT", "Custom File 3", "Custom File 4"};
const string shapeArray[MAX_SHAPES] = {"Square", "Circle"};

const string fileNames[MAX_FILES] = {"amongus.txt", "goat.txt", "contour_output_3.txt", "contour_output_4.txt"};
const string shapeNames[MAX_SHAPES] = {"square.txt", "circle.txt"};

// PID Controller
typedef struct
{
	// Controller gains terms
	float kp;
	float ki;
	float kd;

	// Controller motion profile
	float speed;

	// Derivative low pass filter time constant
	float tau;

	// Output Limits
	float lim_min;
	float lim_max;

	// Sample Time (seconds)
	float sample_time;

	// Controller stored vars
	float integrator;
	float prev_error;
	float differentiator;
	float prev_measurement;

	// Controller Output Var
	float output;

} PID_controller;

// ------ FUNCTION PROTOTYPES ------ //

//UI FUNCTIONS
bool movePointer(int &pointer, int options);
void dispMain(int pointer);
void dispShapes(int pointer);
void dispFiles(int pointer);


// CONVERSION FUNCTIONS
float degrees_to_mm (int degrees, float gear_radius);
float mm_to_degrees (float distance, float gear_radius);
float deg_to_rad(float deg);
float rad_to_deg(float rad);
void pos_mm_to_degree(float* mm_pos, float* deg_pos);
void pos_degree_to_mm(float* mm_pos, float* deg_pos);

// PID CONTROLLER FUNCTIONS
void PID_Controller_reset(PID_controller *pid);
float PID_controller_update(PID_controller *pid, float set_point, float measurement);
void draw_PID(PID_controller* pid_x, PID_controller* pid_y, float* target_pos, bool draw, TFileHandle &fout);


// ROBOTC MOVEMENT FUNCTIONS
void initialize_sensors();
void get_current_pos(float* mm_pos);
void zero();
void move_pen_z(bool move_up);

// ------ FUNCTION DEFINITIONS ------ //
bool movePointer(int &pointer, int options)
{
	while (!getButtonPress(buttonAny))
	{}
	if (getButtonPress(buttonDown))
	{
		if (pointer < options)
			pointer++;
		//loop back to the first option
		else
			pointer = 1;
	}
	//move the currently selected option up
	else if (getButtonPress(buttonUp))
	{
		if (pointer > 1)
			pointer--;
		//loop back to the last option
		else
			pointer = options;
	}
	//set pointer to 0 if user presses enter to exit menu
	else if (getButtonPress(buttonEnter) && pointer == options)
		pointer = 0;
	//return true to go into the desired sub menu
	else if (getButtonPress(buttonEnter) && pointer!= options)
		return true;

	//return false to continue moving the pointer
	return false;
}

void dispMain(int pointer)
{
	displayString(3, "-----ARTICUS MAXIMUS-----");
	displayString(5, "1) Draw from a file");
	displayString(6, "2) Draw a basic shape");
	displayString(7, "3) Exit menu");

	displayString(10, "Currently selected: %d", pointer);
}

//create a
void dispFiles(int pointer)
{
	displayString(3, "Please select a file: ");

	for (int index = 0; index < MAX_FILES; index++)
		displayString(index + 5, "%d) %s", index + 1, fileArray[index]);

	displayString(MAX_FILES + 5, "%d) Go back to main menu", MAX_FILES + 1);
	displayString(12, "Currently selected: %d", pointer);
}

void dispShapes(int pointer)
{
	displayString(3, "Please select a basic shape: ");

	for (int index = 0; index < MAX_SHAPES; index++)
		displayString(index + 5, "%d) %s", index + 1, shapeArray[index]);

	displayString(MAX_SHAPES + 5, "%d) Go back to main menu", MAX_SHAPES + 1);
	displayString(12, "Currently selected: %d", pointer);
}
// Degrees gear turned to linear distance (mm)
float degrees_to_mm (int degrees, float gear_radius)
{
	return degrees * gear_radius * PI / 180;
}
// Linear distance (mm) to degrees for gear turning
float mm_to_degrees (float distance, float gear_radius)
{
	return distance * 180 / gear_radius / PI;
}
// Degrees to radians
float deg_to_rad(float deg)
{
	return deg*PI/180.0;
}
// Radians to degrees
float rad_to_deg(float rad)
{
	return rad/PI*180.0;
}
// Converts pos(x,y) from mm to degrees, returns by reference
void pos_mm_to_degree(float* mm_pos, float* deg_pos)
{
	deg_pos[0] = mm_to_degrees(mm_pos[0], GEAR_RADIUS_X);
	deg_pos[1] = mm_to_degrees(mm_pos[1], GEAR_RADIUS_Y);
}

void pos_degree_to_mm(float* mm_pos, float* deg_pos) {
	mm_pos[0] = degrees_to_mm(deg_pos[0], GEAR_RADIUS_X);
	mm_pos[1] = degrees_to_mm(deg_pos[1], GEAR_RADIUS_Y);
}

// Initialize Touch Sensors
void initialize_sensors()
{
	//touch x
	SensorType[S1] = sensorEV3_Touch;
	//touch y
	SensorType[S2] = sensorEV3_Touch;

}
// Get current pen position in mm
void get_current_pos(float* mm_pos)
{
	float deg_pos[2] = {0, 0};
	deg_pos[0] = -nMotorEncoder[motorA];
	deg_pos[1] = -nMotorEncoder[motorD];
	pos_degree_to_mm(mm_pos, deg_pos);
}
// Zero pen x, y, z
void zero()
{
	int const speed_initial = 20;
	int const speed_final = 10;
	//testing if on the sensor initially and moving off the sensor
	if (SensorValue[S1] == 1)
	{
		motor[motorA] = -speed_initial;
		wait1Msec(250);
		motor[motorA] = 0;
	}

	if (SensorValue[S2] == 1)
	{
		motor[motorD] = -speed_initial;
		wait1Msec(250);
		motor[motorD] = 0;
	}
	//move x and y until they hit the touch sensor
	//x movement
	motor[motorA] = speed_initial;
	while (!SensorValue[S1])
	{}
	motor[motorA] = 0;
	wait1Msec(50);
	motor[motorA] = -speed_final;
	while (SensorValue[S1])
	{}
	motor[motorA] = 0;
	motor[motorA] = speed_final;
	while (!SensorValue[S1])
	{}
	motor[motorA] = 0;
	//y movement
	motor[motorD] = speed_initial;
	while (!SensorValue[S2])
	{}
	motor[motorD] = 0;
	wait1Msec(50);
	motor[motorD] = -speed_final;
	while (SensorValue[S2])
	{}
	motor[motorD] = 0;
	motor[motorD] = speed_final;
	while (!SensorValue[S2])
	{}
	motor[motorD] = 0;

	// Set motor encoder positioning
	nMotorEncoder[motorA] = 0;
	nMotorEncoder[motorD] = 0;
}
// Moves pen up or down to page
void move_pen_z(bool move_up)
{
	if (move_up)
	{
		// Setting motor to run forwards until distance is pen distance away from the page
		motor[motorB] = -25;
		//wait1MSec(100);
		while(0 < nMotorEncoder[motorB])
		{}
		motor[motorB] = 0;
	}
	else
	{
		// Setting motor runs backwards until distance is backwards to 0mm
		motor[motorB] = 25;
		while(40 > nMotorEncoder[motorB])
		{}
		motor[motorB] = 0;
	}
}

// Printing PID Controller values
void PID_controller_log(PID_controller *pid, TFileHandle &fout)
{
	writeTextPC(fout, "integrator");
	writeFloatPC(fout, pid->integrator);

	writeTextPC(fout, " prev_error");
	writeFloatPC(fout, pid->prev_error);

	writeTextPC(fout, " differentiator");
	writeFloatPC(fout, pid->differentiator);

	writeTextPC(fout, " prev_measurement");
	writeFloatPC(fout, pid->prev_measurement);

	writeTextPC(fout, " output");
	writeFloatPC(fout, pid->output);

	writeEndlPC(fout)
}
// Reset PID values
void PID_Controller_reset(PID_controller *pid)
{
	// Reset variables
	pid->integrator = 0.0;
	pid->prev_error = 0.0;
	pid->differentiator = 0.0;
	pid->prev_measurement = 0.0;
	pid->output = 0.0;
}

// Update PID values
float PID_controller_update(PID_controller *pid, float set_point, float measurement)
{
	// Error Signal
	float error = set_point - measurement;

	// Proportional
	float proportional = pid->kp * error;

	// Integral
	pid->integrator += 0.5 * pid->ki * pid->sample_time * (error + pid->prev_error);

	/*
	// Anti-Windup with dynamic integrator clamping
	float lim_min_int = 0, lim_max_int = 0;

	if (pid->lim_max > proportional)
	{
	lim_max_int = pid->lim_max - proportional;
	}
	if (pid->lim_min < proportional)
	{
	lim_min_int = pid->lim_min - proportional;
	}

	// Limit integrator value
	if (pid->integrator < lim_min_int)
	{
	pid->integrator = lim_min_int;
	}
	if (pid->integrator > lim_max_int)
	{
	pid->integrator = lim_min_int;
	}
	*/
	// Derivative (low pass filter)
	pid->differentiator = (2.0 * pid->kd *(measurement - pid->prev_measurement) + (2.0 * pid->tau - pid->sample_time) * pid->differentiator) / (2.0 * pid->tau + pid->sample_time);

	// Compute output
	pid->output = proportional + pid->integrator + pid->differentiator;
	if (pid->output < pid->lim_min)
	{
		pid->output  = pid->lim_min;
	}
	else if (pid->output > pid->lim_max)
	{
		pid->output  = pid->lim_max;
	}

	pid->prev_error = error;
	pid->prev_measurement = measurement;

	return (int)pid->output;
}


void draw_PID(PID_controller* pid_x, PID_controller* pid_y, float* target_pos, bool draw, TFileHandle &fout)
{
	/* Controls x motor and y motor to move pen from starting position
	to ending position using PID controller.

	PARAMETERS
	----------
	pid_x: PID Controller struct for x-direction motor
	pid_y: PID Controller struct for y-direction motor
	target_pos: float array[x,y], target position.
	max_draw_power: int <100, maximum motor power allowed while drawing
	max_move_power: int <100, maximum motor power allowed while moving

	RETURNS
	-------
	*/


	// Pen move until within 0.1mm of actual target
	float const POS_TOL = 1;

	// Initialize starting positions
	float starting_pos[2] = {0, 0};
	float current_pos[2] = {0, 0};
	get_current_pos(current_pos);
	starting_pos[0] = current_pos[0];
	starting_pos[1] = current_pos[1];

	// Motion profile
	float x_f = target_pos[0];
	float x_i = starting_pos[0];
	float y_f = target_pos[1];
	float y_i = starting_pos[1];

	if (draw == false)
	{
		move_pen_z(true);
	}

	// PID Loop
	time1[T1] = 0;
	time1[T2] = 0;
	if (draw == true)
	{
		while ((abs(current_pos[0] - target_pos[0]) > POS_TOL) || (abs(current_pos[1] - target_pos[1]) > POS_TOL))
		{
			// get next point on motion profile
			float t = time1[T1] * pid_x->speed;
			float x_t = x_i + (x_f - x_i) * t;
			float y_t = y_i + (y_f - y_i) * t;

			if (x_i > x_f && x_f - x_t > 0)
			{
				x_t = x_f;
				writeTextPC(fout, "PASSED TARGETx1");
			}
			else if (x_i < x_f && x_t - x_f > 0)
			{
				x_t = x_f;
				writeTextPC(fout, "PASSED TARGETx2");
			}

			if (y_i > y_f && y_f - y_t > 0)
			{
				y_t = y_f;
				writeTextPC(fout, "PASSED TARGETy1");
			}
			else if (y_i < y_f && y_t - y_f > 0)
			{
				y_t = y_f;
				writeTextPC(fout, "PASSED TARGETy2");
			}

			writeTextPC(fout, "target position: ");
			writeFloatPC(fout, x_t);
			writeTextPC(fout, " ");
			writeFloatPC(fout, y_t);
			writeEndlPC(fout);

			// update PID controllers
			PID_controller_update(pid_x, x_t, current_pos[0]);
			PID_controller_update(pid_y, y_t, current_pos[1]);
			writeTextPC(fout, "mA");
			writeFloatPC(fout, -pid_x->output);
			writeTextPC(fout, " mD");
			writeFloatPC(fout, -pid_x->output);
			motor[motorA] = -pid_x->output;
			motor[motorD] = -pid_y->output;

			wait1Msec(pid_x->sample_time);
			float run_t = time1[T2];
			//writeTextPC(fout, " time:");
			//writeFloatPC(fout, run_t);
			// update current position for loop condition
			get_current_pos(current_pos);
			PID_controller_log(pid_x, fout);
			motor[motorA] = motor[motorD] = 0;
		}
	}
	else
	{
		// Move x motor until target
		move_pen_z(true);

		if (current_pos[0] < target_pos[0])
		{
			motor[motorA] = -20;
		}
		else if (current_pos[0] > target_pos[0])
		{
			motor[motorA] = 20;
		}
		else
		{
			motor[motorA] = 0;
		}

		while ((abs(current_pos[0] - target_pos[0]) > POS_TOL))
		{
			// Debug
			//displayString(5, "%f target position", actual_target[0]);
			//displayString(7, "%f current position", current_pos[0]);
			//displayString(9, "%f DIFFERENCE", abs(current_pos[0] - actual_target[0]));
			get_current_pos(current_pos);
		}
		motor[motorA] = 0;

		// Move y motor until target
		if (current_pos[1] < target_pos[1])
		{
			motor[motorD] = -20;
		}
		else if (current_pos[1] > target_pos[1])
		{
			motor[motorD] = 20;
		}
		else
		{
			motor[motorD] = 0;
		}

		while ((abs(current_pos[1] - target_pos[1]) > POS_TOL))
		{
			// Debug
			//displayString(5, "%f target position", actual_target[1]);
			//displayString(7, "%f current position", current_pos[1]);
			//displayString(9, "%f DIFFERENCE", abs(current_pos[1] - actual_target[1]));
			get_current_pos(current_pos);
		}
		motor[motorD] = 0;
		move_pen_z(false);
	}
	return;
}



void draw_pid_from_file(string file_name)
{
	// Initialize position and zero pen
	zero();
	move_pen_z(false);
	// Create controller
	PID_controller pid_x;
	PID_controller pid_y;
	writeTextPC(fout, "PID controllers created");
	writeEndlPC(fout);

	// TODO: Tune Low-pass filter tau and calculate sample time
	pid_x.sample_time = pid_y.sample_time = 10; //ms
	pid_x.speed = pid_y.speed = 0.001;
	pid_x.tau = pid_y.tau = 0.4;
	pid_x.lim_min = pid_y.lim_min = -70.0;
	pid_x.lim_max = pid_y.lim_max = 70.0;
	writeTextPC(fout, "PID values assigned");
	writeEndlPC(fout);

	// TODO: Tune Constants
	pid_x.kp = 5;
	pid_x.ki = 0.01;
	pid_x.kd = 1;
	pid_y.kp = 5;
	pid_y.ki = 0.01;
	pid_y.kd = 1;
	writeTextPC(fout, "PID k-values assigned");
	writeEndlPC(fout);

	// ---- DRAWING LOOP ---- //
	// Read each point
	string move_or_draw = "";
	// Input File Validation
	TFileHandle fin;
	bool fileOkay = openReadPC(fin, file_name);
	if (!fileOkay) {
		displayString(5, "FILE READ ERROR!");
		wait1Msec(3000);
		return;
	}
	while (readTextPC(fin, move_or_draw))
	{
		writeTextPC(fout, "File Read OK, starting drawing");
		writeEndlPC(fout);

		// Determine if D (draw) or M (move)
		bool is_draw = false;
		if (move_or_draw == "D")
		{
			is_draw = true;
		}

		// Get target location
		float next_point[2] = {0,0};
		readFloatPC(fin, next_point[0]);
		readFloatPC(fin, next_point[1]);

		// Move to target location
		writeTextPC(fout, "Moving to point: ");
		writeFloatPC(fout, next_point[0]);
		writeTextPC(fout, " ");
		writeFloatPC(fout, next_point[1]);
		writeEndlPC(fout);
		draw_PID(&pid_x, &pid_y, next_point, is_draw, fout);
		PID_Controller_reset(pid_x);
		PID_Controller_reset(pid_y);
	}
	// close file
	closeFilePC(fin);
}

task main()
{
	openWritePC(fout, "debug_output.txt");
	string file_name = "amongus.txt";
	draw_pid_from_file(file_name);
	closeFilePC(fout);
	move_pen_z(true);
	zero();
}

/*
task main()
{
//delay time for EV3 in milleseconds
const int HOLDTIME = 300;

initialize_sensors();

//initializing option pointer
int pointer = 1;
//number of actions in main menu
int main_option = 3;

while (pointer != 0)
{
bool sub_menu = false;
dispMain(pointer);
sub_menu = movePointer(pointer, main_option);
if (sub_menu)
{
int sub_pointer = 1;
eraseDisplay();
wait1Msec(HOLDTIME);
bool run_code = false;
while(sub_pointer != 0)
{
int sub_options = 0;

if(pointer == 1)
{
sub_options = MAX_FILES + 1;
dispFiles(sub_pointer);
}

else
{
sub_options = MAX_SHAPES + 1;
dispShapes(sub_pointer);
}

run_code = movePointer(sub_pointer, sub_options);

if(run_code)
{
eraseDisplay();
wait1Msec(HOLDTIME);

if(pointer == 1)
{
string temp_file = fileNames[sub_pointer - 1];
draw_pid_from_file(temp_file);
displayString(7, "Press enter to return back");
while(!getButtonPress(buttonEnter))
{}
}

else
{
string temp_file = shapeNames[sub_pointer - 1];
draw_pid_from_file(temp_file);
displayString(7, "Press enter to return back");
while(!getButtonPress(buttonEnter))
{}
}
eraseDisplay();
}
wait1Msec(HOLDTIME);
} //while ends
eraseDisplay();
wait1Msec(HOLDTIME);
}
wait1Msec(HOLDTIME);
}
}
*/


/*
void draw_image_from_file_no_PID(string file_name)
{
int const MAX_DRAW_POWER = 13;
int const MAX_MOVE_POWER = 20;
// Input File Validation
TFileHandle fin;
bool fileOkay = openReadPC(fin, file_name);
if (!fileOkay) {
displayString(5, "FILE READ ERROR!");
wait1Msec(3000);
return;
}

// Initialize position and zero pen
zero();
move_pen_z(false);

// ---- DRAWING LOOP ---- //
// Read line-by-line
string move_or_draw;
while (readTextPC(fin, move_or_draw))
{
// Read next point
float next_point[2] = {0,0};
readFloatPC(fin, next_point[0]);
readFloatPC(fin, next_point[1]);

// Update boolean move or draw depending on input
bool is_draw = false;
if (move_or_draw == "D")
{
is_draw = true;
}

// Move to point
displayString(5, "drawing points");
draw_no_PID(next_point, is_draw, MAX_DRAW_POWER, MAX_MOVE_POWER);
writeTextPC(fout, move_or_draw);
writeTextPC(fout, " ");
writeFloatPC(fout, next_point[0]);
writeTextPC(fout, " ");
writeFloatPC(fout, next_point[1]);
writeEndlPC(fout);
}
// close file
closeFilePC(fin);
move_pen_z(true);
zero();
}
*/
