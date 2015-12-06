#include "nr3.h"
#include "odeint.h"
#include "stepper.h"
#include "stepperdopr5.h"

using namespace std;


//64 cells wide (for distance)
//
struct RoadRage {

	VecDoub rho;
	Doub u_max,p_max, size_road;

	//constructor (get and set rho vector)
	RoadRage(Doub u_max_in, Doub p_max_in,Doub size_road_in){
		 u_max = u_max_in;
		 p_max = p_max_in;
		 size_road = size_road_in;

		};

	~RoadRage() {};

	void operator() (const Doub x, VecDoub_I &rho, VecDoub_O &dpdt){

		Doub f_right,f_left;
		Doub delta = 1.0 / size_road;

		for(int i = 0; i <= size_road; i++){
			Doub p = rho[i];

			//Boundaries
			//Left boundary
			if(i==0) {
				f_left = rho[size_road] * u_max * (1.0 - rho[size_road]/ p_max);
				f_right = rho[i+1] * u_max * (1.0 - rho[i+1])/ p_max;
			} 
			//Right boundary
			else if (i==size_road) {
				f_right = rho[0] * u_max * (1.0 - rho[0]/ p_max);
				f_left = rho[i-1] * u_max * (1.0 - rho[i-1]/ p_max);
			} else{
				f_right = rho[i+1] * u_max * (1.0 - rho[i+1]/ p_max);
				f_left = rho[i-1] * u_max * (1.0 - rho[i-1]/ p_max);
			}

			//Find dpdt
			dpdt[i] = (f_right - f_left) / (2*delta);


		}
	}

};

struct Traffic{
	const static Doub x_center = 0.5;
	const static Doub lambda = 0.1;
	const static Doub u_max = 1.0;
	const static Doub p_max = 1.0;
	const static Doub x_max = 1.0;



	Doub perturb;
	Doub rho_bar;

	int dim;

	MatDoub rho_matrix;
	VecDoub road;
	Doub size_road = 8;
	Doub t = 5;

	Doub delta = 1.0/ (Doub)size_road;



	Traffic(Doub perturb_in, Doub rho_bar_in) {
		perturb = perturb_in * p_max;
		rho_bar = rho_bar_in * p_max;
	}

	void init(){

		rho_matrix.assign(t,size_road + 1, 1.0);
		road.assign((int)size_road + 1, 1.0);


		//initalize values at t = 0
		int index = 0;

		for(Doub x = 0.0; x < x_max; x += delta){
			
			Doub cur_rho = rho_bar + perturb
				* exp(-( x - x_center)* ( x - x_center) / (lambda * lambda));

			road[index] = cur_rho;
			rho_matrix[0][index] = cur_rho;

			index++;
		}
		print_road();
	}

	void update(){

		print_road();
		cout << road[0];
		//sets absolute and relative tolerances
    	//also sets minimum step size and first guess of step size
   		const Doub atol = 1.e-5, rtol = atol, h1=0.001, hmin=0.0;

   		Output out;


		RoadRage anger(u_max, p_max, size_road);



		for(int i = 0; i < t; i++){
			Odeint<StepperDopr5<RoadRage> > ode(road, i, i+1, atol, rtol, h1, hmin,out,anger);
			ode.integrate();

			//cout << rho_matrix[t][0] << endl;
			//cout << road[0];

			for(int s = 0; s <= size_road; s++){
				cout << "hi\n";
				rho_matrix[t][s] = road[s];
				//cout << "hi\n";
			}
		}
	}

	~Traffic(){}

	void print_matrix(){

		for(int i = 0; i < t; i++){
			

			for(int j = 0; j < size_road; j++){

				cout << setw(12) << setprecision(3) << rho_matrix[i][j];

			}
			cout << "\n";
		}
	}

	void print_road(){
		for(int i = 0; i < size_road; i++){
			cout << setw(5) << setprecision(3) << road[i];
		}
		cout << endl;
	}

};



int main(){

	Traffic traf(10e-3, 0.5);

	//traf.print_matrix();
	//traf.print_road();

	traf.init();
	//traf.print_road();
	traf.update();


	traf.print_matrix();
	//traf.print_road();

	return 0;
}