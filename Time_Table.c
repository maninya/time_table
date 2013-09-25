#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define INPUT_FILE "EnrollmentStatusReport_fall2011_v6.csv"
#define FACULTY_FILE "faculty_availability.txt"
#define OUTPUT_FILE "Time_Table.txt"
#define MAX_LINE_SIZE 250
#define MAX_NO_OF_RECORDS 1500
#define MAX_NO_OF_COURSES 100
#define MAX_NO_OF_FACULTY 30
#define MAX_POPULATION_SIZE 3
#define DAYS_IN_WEEK 6
#define HOURS_IN_DAY 4
#define NUMBER_OF_ROOMS 5
#define MAXIMUM_ALLOWED_COST 0



/*************************************************************************************/
/*************************************************************************************/
/********************* STRUCTURE DEFINITIONS START HERE *******************************/ 
/*************************************************************************************/
/*************************************************************************************/



/*======================== STRUCTURE USED TO GET RAW DATA FROM THE INPUT FILE ===========================*/
struct record
{
	char rollno[10];
	char course_id[20];
	char course_name[100];
	char faculty_name[100];
	int faculty_id;
}record_arr[MAX_NO_OF_RECORDS];



/*======================== STRUCTURE USED TO STORE LIST OF DISTINCT FACULTY AND THEIR DETAILS IN AN ARRAY ===========================*/
struct faculty 
{
	char faculty_name[100];
	int availability_time_table[DAYS_IN_WEEK][HOURS_IN_DAY];
	int faculty_id;
	int days_available;
}faculty_arr[MAX_NO_OF_FACULTY];




/*======================== STRUCTURE USED TO STORE THE LIST OF DISTINCT STUDENTS AND THEIR DETAILS IN AN ARRAY ===========================*/

/*------- Here "index" stores the index of the course id in the courses array -----------*/
struct student
{
     char roll_no[10];
     int courses[20];	
	 int index;
}student_arr[MAX_NO_OF_RECORDS];



/*======================== STRUCTURE USED TO STORE THE LIST OF DISTINCT STUDENTS AND THEIR DETAILS IN AN ARRAY ===========================*/

/*-------- Here "class_size" denotes the number of students taking that particular course --------------*/
/*--------The "faculty_reference" denotes the index of the faculty in the faculty array --------------*/
struct course
{
	char course_id[20];
	char course_name[100];
	int class_size;
	int faculty_id;
	int faculty_reference;
}course_arr[MAX_NO_OF_COURSES];



/*======================== STRUCTURE USED TO STORE THE LIST OF DISTINCT CLASS-ROOMS AND THEIR DETAILS IN AN ARRAY ===========================*/

/*---------Here capacity_of_room denotes the no. of students that can be accomodated at a time in the room-----------------*/
struct room
{
	int room_number;
	int capacity_of_room;
}room_arr[NUMBER_OF_ROOMS];



/*======================== STRUCTURE USED TO STORE THE DETAILS OF A 3D TIME-TABLE ========================*/

/*------- Each 3D time table is a set of 2D TIME-TABLES, ONE FOR EACH ROOM------*/

/*------- Each 3D time-table is identified by its total "cost" which is the total no. of constraints it violates --------*/

/*------- The individual constraints are also part of the structure as shown ----------*/
struct time_table 
{
	struct time_table *next;
	int table[NUMBER_OF_ROOMS][DAYS_IN_WEEK][HOURS_IN_DAY];
	int cost;
	int class_clash_error;
	int room_small_error;
	int faculty_double_booked_error;
	int faculty_unavailable_error;
	int class_twice_error;
	//int rooms_different_error;
};




/*======================== STRUCTURE USED TO STORE A COLLECTION OF 3D TIME-TABLES IN THE POPULATION ===========================*/

/*------ It contains the population size, and pointers to the first and last 3D time-tables in the population (which is a linked list) -------*/

/*------ It also contains the average and individual costs for each population (ie for the whole linked list of 3D time-tables ------*/
struct colony 
{
	struct time_table *first_time_table;
	struct time_table *last_time_table;
	int population_size;
	int average_cost;
	int class_clash_error;
	int room_small_error;
	int faculty_double_booked_error;
	int faculty_unavailable_error;
	int class_twice_error;
	int rooms_different_error;
}solution_colony;



/*======================== STRUCTURE USED TO STORE THE CURRENT ARRAY OF CLASS AND ROOM, FOR RELATED CLASSES ===========================*/
struct curr_array
{
	int curr_class;
	int curr_room;
}curr_arr[DAYS_IN_WEEK];




//Two dimensional array for storing related classes
int class_clash_arr[MAX_NO_OF_COURSES][MAX_NO_OF_COURSES];

/*------- Variables that depend on the input file -----------*/

int total_no_of_records;
int total_no_of_faculty;
int total_no_of_courses;
int population_size;
int mutation_rate = 16; 




/*************************************************************************************/
/*************************************************************************************/
/********************* FUNCTION DEFINITIONS START HERE *******************************/ 
/*************************************************************************************/
/*************************************************************************************/



/*==================== FUNCTION TO GET THE RAW DATA FROM FILE INTO THE RECORD ARRAY ===================*/
void parse_file()
{
	char line[MAX_LINE_SIZE];
	FILE *myfile;
	char *stptr;
 	int i=0,j;
	
	if(!(myfile = fopen(INPUT_FILE,"r")))
	{
		fprintf(stderr,"Could not open \"alive.txt\" for reading\n");
		exit (0);
	} 
	fgets(line,sizeof line,myfile);
	while(fgets(line,sizeof line,myfile) != NULL)
	{ 
		stptr = line; 

		char *a = strtok(stptr, "\",");
		strcpy(record_arr[i].rollno,a);
		
		a = strtok(NULL,"\",");
		strcpy(record_arr[i].course_id,a);
		
		a = strtok(NULL,"\",");
		strcpy(record_arr[i].course_name,a);
		
		a = strtok(NULL,"\",");
		int faculty_id=atoi(a);
		record_arr[i].faculty_id=faculty_id;

		a = strtok(NULL,"\",");
		strcpy(record_arr[i].faculty_name,a);
		
		i++;
	}
	
	fclose(myfile);
	total_no_of_records = i;
	printf("\nTotal number of records = %d\n",total_no_of_records);
}



/*==================== FUNCTION TO GET THE FACULTY AVAILABILTY DATA FROM ANOTHER FILE ===================*/
void parse_faculty_availability_file()
{
	FILE *fp;
	if((fp=fopen(FACULTY_FILE,"r"))==NULL)
	{
		printf("\nUnable to open faculty availability file\n");	
		exit(0);
	}	
	int day,hour,flag=0;
	int a,i;
	char name[100];
	
	while(!feof(fp))
	{
		fgets(name,100,fp);
		fscanf(fp,"%d",&a);
		for(i=0;i<total_no_of_faculty;i++)
		{
			if(faculty_arr[i].faculty_id == a)
			{
				flag=1;
				break;
			}
		}
		
		if(flag==1)
		{
			flag=0;
			for(day=0;day<DAYS_IN_WEEK;day++)
			{
				for(hour=0;hour<HOURS_IN_DAY;hour++)
				{
					fscanf(fp,"%d",&a);
					faculty_arr[i].availability_time_table[day][hour] = a;
				}
			}
		}
		else
		{
			for(day=0;day<DAYS_IN_WEEK;day++)
			{
				for(hour=0;hour<HOURS_IN_DAY;hour++)
				{
					fscanf(fp,"%d",&a);
				}
			}
		}
	}
	fclose(fp);
}

/*==================== FUNCTION TO CREATE THE FACULTY ARRAY OF STRUCTURES FROM RECORD ARRAY ===================*/

void make_faculty_structure()
{
	int i,j,k=1,count=0,flag=0;
	
	strcpy(faculty_arr[0].faculty_name,record_arr[0].faculty_name);
	faculty_arr[0].faculty_id = record_arr[0].faculty_id;
         
	for(i=1;i<total_no_of_records;i++)
	{	
		flag=0;
		for(j=0;j<k;j++)
		{
			if(record_arr[i].faculty_id == faculty_arr[j].faculty_id)
			{ 
				flag=1;
			}
		}
		if(flag!=1)
		{
			strcpy(faculty_arr[k].faculty_name,record_arr[i].faculty_name);
			faculty_arr[k].faculty_id = record_arr[i].faculty_id;
			count++;
			k++;
		}
	}
	total_no_of_faculty = count+1;
	printf("\nTotal number of faculty = %d\n",total_no_of_faculty);
	
	for(i=0;i<total_no_of_faculty;i++)
	{
		faculty_arr[i].days_available=0;
	}
	
	for(i=0;i<total_no_of_faculty;i++)
	{
		for(j=0;j<DAYS_IN_WEEK;j++)
		{
			for(k=0;k<HOURS_IN_DAY;k++)
			{
				faculty_arr[i].availability_time_table[j][k] = 1;
			}
		}	
	}
	
	parse_faculty_availability_file();

	for(i=0;i<total_no_of_faculty;i++)
	{
		for(j=0;j<DAYS_IN_WEEK;j++)
		{
			for(k=0;k<HOURS_IN_DAY;k++)
			{
				if(faculty_arr[i].availability_time_table[j][k] == 1)
				{
					faculty_arr[i].days_available++;
					break;
				}
			}
		}	
	}
}


/*==================== FUNCTION TO CREATE THE COURSE ARRAY OF STRUCTURES FROM RECORD ARRAY ===================*/

/*------------- Here project, thesis and elective courses have not been included -------------*/

void make_course_structure()
{
	int i,j,k=1,l,n,count=0,flag=0;
	
	for(n=0;n<total_no_of_records;n++)
	{
		if(strcmp(record_arr[n].course_name,"Project Elective")==0);
		else if(strcmp(record_arr[n].course_name,"Supervised Reading / Research Elective")==0);
		else if(strcmp(record_arr[n].course_name,"Supervised Reading Research Elective")==0);
		else if(strcmp(record_arr[n].course_name,"Thesis")==0);
		else if(strcmp(record_arr[n].course_name,"Supervised Reading (MS / Ph.D.)")==0);
		else if(strcmp(record_arr[n].course_name,"Thesis/Research credits")==0);
		else if(strcmp(record_arr[n].course_name,"Research Elective")==0);
		else
		{
			strcpy(course_arr[0].course_name,record_arr[n].course_name);
			strcpy(course_arr[0].course_id,record_arr[n].course_id);
			course_arr[0].class_size = 1;
			course_arr[0].faculty_id = record_arr[n].faculty_id;
			for(l=0;l<total_no_of_faculty;l++)
			{
				if(course_arr[0].faculty_id == faculty_arr[l].faculty_id)
				{
					course_arr[0].faculty_reference = l;
					break;
				}
			}
			break;
		}
	}
	
	for(i=n+1;i<total_no_of_records;i++)
	{	
		flag=0;
		for(j=0;j<k;j++)
		{
			if((strcmp(record_arr[i].course_id,course_arr[j].course_id) == 0) && (record_arr[i].faculty_id == course_arr[j].faculty_id))
			{ 
				flag=1;
				course_arr[j].class_size++;
			}
		}
		if(flag!=1)
		{
			if(strcmp(record_arr[i].course_name,"Project Elective")==0);
			else if(strcmp(record_arr[i].course_name,"Supervised Reading / Research Elective")==0);
			else if(strcmp(record_arr[i].course_name,"Supervised Reading Research Elective")==0);
			else if(strcmp(record_arr[i].course_name,"Thesis")==0);
			else if(strcmp(record_arr[i].course_name,"Supervised Reading (MS / Ph.D.)")==0);
			else if(strcmp(record_arr[i].course_name,"Thesis/Research credits")==0);
			else if(strcmp(record_arr[i].course_name,"Research Elective")==0);
			else
			{
				strcpy(course_arr[k].course_id,record_arr[i].course_id);
				strcpy(course_arr[k].course_name,record_arr[i].course_name);
				course_arr[k].faculty_id = record_arr[i].faculty_id;
				for(l=0;l<total_no_of_faculty;l++)
				{
					if(course_arr[k].faculty_id == faculty_arr[l].faculty_id)
					{
						course_arr[k].faculty_reference = l;
						break;
					}
				}
				course_arr[k].class_size = 1;
				count++;
				k++;
			}
		}
	}
	
	total_no_of_courses = count+1;
	printf("\nTotal number of courses = %d\n",total_no_of_courses);
}


/*==================== FUNCTION TO CREATE THE ROOM ARRAY OF STRUCTURES (TOTAL 5 ROOMS) ===================*/
void make_room_structure()
{
	room_arr[0].room_number = 102;
	room_arr[0].capacity_of_room = 40; 
	room_arr[1].room_number = 103;
	room_arr[1].capacity_of_room = 40; 
	room_arr[2].room_number = 132;
	room_arr[2].capacity_of_room = 40; 
	room_arr[3].room_number = 133;
	room_arr[3].capacity_of_room = 40; 
	room_arr[4].room_number = 106;
	room_arr[4].capacity_of_room = 200; 
}


/*=================== THIS FUNCTION FINDS THE RELATED CLASSES =====================*/

/*--------------- If the same set of students take two different courses, then those classes become related classes,
				  and hence cannot be held at the same time, as the same students cannot be in two classes at once --------------------------*/
void find_related_classes()
{
	int i,j,k=1,flag=0,m=0,curr_index;
    strcpy(student_arr[0].roll_no,record_arr[0].rollno);
 	
	for(i=0;i<total_no_of_courses;i++)
	{
		class_clash_arr[i][i] = 1;
	}
	for(i=0;i<total_no_of_records;i++)
	{
		student_arr[i].index = 0;
	}
	for(i=1;i<total_no_of_records;i++)
	{	
		flag=0;
		for(j=0;j<k;j++)	
		{
			if(strcmp(record_arr[i].rollno,student_arr[j].roll_no)==0)
			{
				flag=1;
				for(m=0;m<total_no_of_courses;m++)
				{
					if(strcmp(record_arr[i].course_id,course_arr[m].course_id)==0)
					{
						student_arr[j].courses[student_arr[j].index] = m;
						for(curr_index=0;curr_index<=student_arr[j].index;curr_index++)
						{
							class_clash_arr[m][student_arr[j].courses[curr_index]] = 1;
							class_clash_arr[student_arr[j].courses[curr_index]][m] = 1;
						}
			        	student_arr[j].index++;
					}
				}
			}
		}
		if(flag!=1)
		{
			strcpy(student_arr[k].roll_no,record_arr[i].rollno);
			for(m=0;m<total_no_of_courses;m++)
			{
				if(strcmp(record_arr[i].course_id,course_arr[m].course_id)==0)
				{
					student_arr[k].courses[0]=m;
					student_arr[k].index++;
				}
			}
			k++;
		}
	}
}


/*================== FUNCTIONS TO ENSURE THAT ALL CLASSES APPEAR EXACTLY ONCE AT A LOCATION ====================*/
/*---------- Done in 2 stages repair_0 and repair_1 -------------------------*/



/*------------ Any classes which appear more than once
	are (non deterministically) altered such that they appear only once ----------------------*/
void repair_0(struct time_table *curr_ptr)
{
	struct booking_location 
	{
		int which_room;
		int which_day;
		int which_hour;
		struct booking_location *next;
	};
	struct booking_location *first;
	struct booking_location *temp1;
	struct booking_location *temp2;
	struct booking_location *curr_booking;
	struct booking_location *booking_to_remove;
	int curr_class;
	int class_occurred;
	int room;
	int day;
	int hour;
	int one_to_remove;
	int i = 0;
	
	for(curr_class=0;curr_class<total_no_of_courses;curr_class++)
	{
		class_occurred = 0;
		for(room=0;room<NUMBER_OF_ROOMS;room++)
		{
			for(day=0;day<DAYS_IN_WEEK;day++)
			{
				for(hour=0;hour<HOURS_IN_DAY;hour++)
				{
					if(curr_ptr -> table[room][day][hour] == curr_class) 
					{
						class_occurred++;
						if(class_occurred == 1) 
						{
							if(((first)=(struct booking_location *)malloc(sizeof(struct booking_location)))==NULL) 
							{
								printf("\nInsufficient memory for booking\n");
								exit(0);
							}
							curr_booking = first;
						}
						else 
						{
							if(((curr_booking->next)=(struct booking_location *)malloc(sizeof(struct booking_location)))==NULL) 
							{
								printf("\nInsufficient memory for booking\n");
								exit(0);
							}
							curr_booking = curr_booking -> next;
						}
						curr_booking -> which_room = room;
						curr_booking -> which_day = day;
						curr_booking -> which_hour = hour;
						curr_booking -> next = NULL;
					}
				}
			}
		}
		if(class_occurred == 1 || class_occurred == 2 ) 
		{
			temp1=first;
			while(temp1!=NULL)
			{
				temp2=temp1;
				temp1=temp1->next;
				free(temp2);
			}
		}
		else
		{
			if(class_occurred > 2) 
			{
				while(class_occurred > 2) 
				{
					curr_booking = first;
					one_to_remove = random()%class_occurred;
					for(i=0;i<one_to_remove;i++) 
					{
						curr_booking = curr_booking -> next;
					}
					room = curr_booking -> which_room;
					day = curr_booking -> which_day;
					hour = curr_booking -> which_hour;
					if(curr_ptr -> table[room][day][hour] == curr_class) 
					{
						curr_ptr -> table[room][day][hour] = -1;
						if(one_to_remove == 0) 
						{
							booking_to_remove = first;
							first = first -> next;
						}
						else
						{
							if(one_to_remove != class_occurred)
							{
								curr_booking = first;
								for(i=0;i<(one_to_remove-1);i++)
								{
									curr_booking = curr_booking -> next;
								}
								booking_to_remove = curr_booking -> next;
								curr_booking -> next = (curr_booking -> next)-> next;
							}
						}
						free(booking_to_remove);
						class_occurred --;
					}
				}
				temp1=first;
				while(temp1!=NULL)
				{
					temp2=temp1;
					temp1=temp1->next;
					free(temp2);
				}
			}
		}
	}
}


/*------------------ Any classes which did not appear at all are booked to a spare
						space (regardless of room size, etc) -----------------------*/

/*----------------- This strategy is also used to initialise a random population --------------------*/

void repair_1(struct time_table *curr_ptr)
{
	int curr_class;
	int class_occurred;
	int room;
	int day;
	int hour;
	
	for(curr_class=0;curr_class<total_no_of_courses;curr_class++) 
	{
		class_occurred = 0;
		for(room=0;room<NUMBER_OF_ROOMS;room++)
		{
			for(day=0;day<DAYS_IN_WEEK;day++)
			{
				for(hour=0;hour<HOURS_IN_DAY;hour++) 
				{
					if(curr_ptr -> table[room][day][hour] == curr_class) 
					{
						class_occurred++;
					}
				}
			}
		}
		if (class_occurred == 0 || class_occurred == 1 ) 
		{
			while(class_occurred != 2) 
			{
				room = random()%NUMBER_OF_ROOMS;
				day = random()%DAYS_IN_WEEK;
				hour = random()%HOURS_IN_DAY;
				if (curr_ptr ->	table[room][day][hour] == -1) 
				{
					curr_ptr ->	table[room][day][hour] = curr_class;
					class_occurred ++;
				}
			}
		}
	}
}

/*
int rooms_different(struct time_table *curr_ptr)
{
	int i,day,room,hour,room1;
	int num_of_occurrences = 0,flag = 0;
	
	for(i=0;i<total_no_of_courses;i++)
	{
		for(day=0;day<DAYS_IN_WEEK;day++)
		{
			for(room=0;room<NUMBER_OF_ROOMS;room++)
			{
				for(hour=0;hour<HOURS_IN_DAY;hour++)
				{
					if(curr_ptr -> table[room][day][hour] == i)
					{
						if(flag==0)
						{
							flag=1;
							room1=room;
						}
						else
						{
							flag=2;
							break;
						}
					}
				}
				if(flag==2)
				{
					break;
				}
			}
			if(flag==2)
			{
				flag=0;
				break;
			}
		}
		if(room1 != room)
		{
			num_of_occurrences++;
		}
	}
	return num_of_occurrences;
}
*/

/*===================== FUNCTION TO COUNT THE NUMBER OF TIMES A CLASS IS BOOKED TWICE IN THE SAME ROOM ===================*/

int class_twice(struct time_table *curr_ptr)
{ 
    int i,num_of_occrurences = 0;
	int room;
	int day;
	int hour,curr_room,curr_hour;
         
	for(room=0;room<NUMBER_OF_ROOMS;room++) 
	{
		for(day=0;day<DAYS_IN_WEEK;day++) 
		{
			for(hour=0;hour<HOURS_IN_DAY;hour++) 
			{	
				for(curr_room=0;curr_room<NUMBER_OF_ROOMS;curr_room++) 
				{
					for(curr_hour=0;curr_hour<HOURS_IN_DAY;curr_hour++) 
					{
						if((curr_ptr -> table[room][day][hour] == curr_ptr -> table[curr_room][day][curr_hour]) && (room!= curr_room || hour!=curr_hour) && (curr_ptr -> table[room][day][hour] != -1) && (faculty_arr[course_arr[curr_ptr -> table[room][day][hour]].faculty_reference].days_available != 1))
						{
							num_of_occrurences ++;
						}
					}
				}
			}
		}
	}
	return num_of_occrurences ;
}


/*===================== FUNCTION TO COUNT THE NUMBER OF TIMES A CLASS IS BOOKED TWICE AT THE SAME TIME ===================*/

int class_clash(struct time_table *curr_ptr)
{
	int i,num_of_occurrences = 0;
	int room;
	int day;
	int hour;
	int curr_class1,curr_class2;
	
	for(day=0;day<DAYS_IN_WEEK;day++) 
	{
		for(hour=0;hour<HOURS_IN_DAY;hour++) 
		{
			for (room=0;room<NUMBER_OF_ROOMS-1;room++) 
			{
				curr_class1 = curr_ptr -> table[room][day][hour];
				if(curr_class1 != -1)
				{
					for(i=room+1;i<NUMBER_OF_ROOMS;i++)
					{
						curr_class2 = curr_ptr -> table[i][day][hour];
						if(curr_class2 != -1)
						{
							if(class_clash_arr[curr_class1][curr_class2] == 1) 
							{
								num_of_occurrences++;
							}
						}
					}
				}
			}
		}
	}
	return num_of_occurrences;
}

/*===================== FUNCTION TO COUNT THE NUMBER OF TIMES A CLASS IS BOOKED TO A ROOM TOO SMALL FOR IT ===================*/

int room_small(struct time_table *curr_ptr)
{
	int num_of_occurrences = 0;
	int curr_room;
	int curr_size_available;
	int curr_size_allocated;
	int curr_day;
	int curr_class;
	int hour;
	
	for(curr_room=0;curr_room<NUMBER_OF_ROOMS;curr_room++)
	{
		curr_size_available =room_arr[curr_room].capacity_of_room;
		for(curr_day=0;curr_day<DAYS_IN_WEEK;curr_day++)
		{
			for(hour=0;hour<HOURS_IN_DAY;hour++)
			{
				curr_class = curr_ptr -> table[curr_room][curr_day][hour];
				if(curr_class != -1) 
				{
					curr_size_allocated = course_arr[curr_class].class_size;
					if(curr_size_available < curr_size_allocated) 
					{
						num_of_occurrences ++;
					}
				}
			}
		}
	}
	return num_of_occurrences;
}

/*===================== FUNCTION TO COUNT THE NUMBER OF TIMES A FACULTY MEMBER IS DOUBLE-BOOKED ===================*/

int faculty_double_booked(struct time_table *curr_ptr)
{
	int num_of_occurrences = 0;
	int curr_room;
	int faculty_num;
	int curr_faculty;
	int num_of_bookings_at_this_time;
	int curr_class;
	int curr_class_num;
	int curr_day;
	int hour;
	
	for(faculty_num=0;faculty_num<total_no_of_faculty;faculty_num++) 
	{
		for(curr_day=0;curr_day<DAYS_IN_WEEK;curr_day++)
		{
			for(hour=0;hour<HOURS_IN_DAY;hour++)
			{
				num_of_bookings_at_this_time = 0;
				for(curr_room=0;curr_room<NUMBER_OF_ROOMS;curr_room++)
				{
					curr_class_num = (curr_ptr -> table[curr_room][curr_day][hour]);
					if(curr_class_num != -1)
					{
						curr_faculty = course_arr[curr_class_num].faculty_reference;
						if(faculty_num == curr_faculty) 
						{
							num_of_bookings_at_this_time++;
						}
					}
					if(num_of_bookings_at_this_time>1) 
					{
						num_of_occurrences += (num_of_bookings_at_this_time-1);
					}
				}
			}
		}
	}
	return num_of_occurrences;
}

/*===================== FUNCTION TO COUNT THE NUMBER OF TIMES A FACULTY MEMBER IS BOOKED WHEN UNAVAILABLE ===================*/
int faculty_unavailable(struct time_table *curr_ptr)
{
	int num_of_occurrences = 0;
	int curr_room;
	int faculty_num;
	int curr_faculty;
	int curr_class;
	int curr_class_num;
	int curr_day;
	int hour;

	for(curr_day=0;curr_day<DAYS_IN_WEEK;curr_day++)
	{
		for(hour=0;hour<HOURS_IN_DAY;hour++)
		{
			for(curr_room=0;curr_room<NUMBER_OF_ROOMS;curr_room++)
			{
				curr_class_num = (curr_ptr -> table[curr_room][curr_day][hour]);
				if(curr_class_num != -1) 
				{
					curr_faculty = course_arr[curr_class_num].faculty_reference;
					if(faculty_arr[curr_faculty].availability_time_table[curr_day][hour] == 0) 
					{
						num_of_occurrences++;
					}
				}
			}
		}
	}
	return num_of_occurrences;
}


/*===================== FUNCTION TO FIND THE TOTAL COST OF ERRORS FOR A TIME-TABLE ===================*/
void calculate_cost(struct time_table *curr_ptr)
{
	curr_ptr -> class_clash_error = class_clash(curr_ptr);
	curr_ptr -> room_small_error = room_small(curr_ptr);
	curr_ptr -> faculty_double_booked_error = faculty_double_booked(curr_ptr);
	curr_ptr -> faculty_unavailable_error = faculty_unavailable(curr_ptr);
	curr_ptr -> class_twice_error = class_twice(curr_ptr);
	//curr_ptr -> rooms_different_error = rooms_different(curr_ptr);
	
	curr_ptr -> cost = 0;
	curr_ptr -> cost += curr_ptr -> class_clash_error; 
	curr_ptr -> cost += curr_ptr -> room_small_error;
	curr_ptr -> cost += curr_ptr -> faculty_double_booked_error;
	curr_ptr -> cost += curr_ptr -> faculty_unavailable_error;
	curr_ptr -> cost += curr_ptr -> class_twice_error;
	//curr_ptr -> cost += curr_ptr -> rooms_different_error;	
}


/*===================== FUNCTION TO FIND THE AVERAGE COST OF ERRORS FOR A TIME-TABLE ===================*/
void find_average_cost()
{
	int sum_of_costs = 0;
	int sum1 = 0;
	int sum2 = 0;
	int sum3 = 0;
	int sum4 = 0;
	int sum5 = 0;
	//int sum6 = 0;
	struct time_table *curr_ptr;

	curr_ptr = solution_colony.first_time_table;

	while (curr_ptr != NULL) 
	{
		sum_of_costs += curr_ptr -> cost;
		sum1 += curr_ptr -> class_clash_error;
		sum2 += curr_ptr -> room_small_error;
		sum3 += curr_ptr -> faculty_double_booked_error;
		sum4 += curr_ptr -> faculty_unavailable_error;
		sum5 += curr_ptr -> class_twice_error;
		//sum6 += curr_ptr -> rooms_different_error;
		curr_ptr = curr_ptr-> next;
	}
	solution_colony.average_cost = (sum_of_costs / (solution_colony.population_size));
	solution_colony.class_clash_error = (sum1 / (solution_colony.population_size));
	solution_colony.room_small_error = (sum2 / (solution_colony.population_size));
	solution_colony.faculty_double_booked_error = (sum3 / (solution_colony.population_size));
	solution_colony.faculty_unavailable_error = (sum4 / (solution_colony.population_size));
	solution_colony.class_twice_error = (sum5 / (solution_colony.population_size));
	//solution_colony.rooms_different_error = (sum6 / (solution_colony.population_size));
}


/*===================== FUNCTION TO POPULATE THE COLONY INITIALLY WITH A RANDOM SET OF TIME-TABLES, USES REPAIR STRATEGY 1 ===================*/

void initialise_colony()
{
	struct time_table *curr_ptr;
	struct time_table *test_ptr;
	int room,day,hour;
	int final_population_size;

	solution_colony.population_size = 0;
	final_population_size = population_size;

	while(solution_colony.population_size < final_population_size)
	{
		if(((test_ptr) = (struct time_table *)malloc(sizeof(struct time_table))) == NULL)
		{
			printf("\nInsufficient memory for Time Table allocation \n");
			exit(0);
		}
		if(test_ptr -> next != NULL) 
		{
			test_ptr -> next = NULL;
		}
		for(room=0;room<NUMBER_OF_ROOMS;room++)
		{
			for(day=0;day<DAYS_IN_WEEK;day++)
			{
				for(hour=0;hour<HOURS_IN_DAY;hour++)
				{
					test_ptr -> table[room][day][hour] = -1;
				}
			}
		}
		repair_1(test_ptr);
		calculate_cost(test_ptr);
		if(solution_colony.population_size == 0) 
		{
			solution_colony.first_time_table = test_ptr;
			solution_colony.last_time_table = test_ptr;
			solution_colony.population_size++;
		} 
		else 
		{
			curr_ptr = solution_colony.first_time_table;
			if(curr_ptr -> cost >= test_ptr -> cost) 
			{
				test_ptr -> next = curr_ptr;
				solution_colony.first_time_table = test_ptr;
				solution_colony.population_size++;
			}
			else 
			{
				curr_ptr = solution_colony.first_time_table;
				while((curr_ptr -> next != NULL) && ((curr_ptr -> next)-> cost <= test_ptr -> cost))
				{
					curr_ptr = curr_ptr -> next;
				}
				if(curr_ptr == solution_colony.last_time_table) 
				{
					solution_colony.last_time_table	= test_ptr;
					test_ptr -> next = NULL;
				}
				else
				{
					test_ptr -> next = curr_ptr -> next;
				}
				curr_ptr->next = test_ptr;
				solution_colony.population_size++;
			}
		}
	}
}

/*===================== FUNCTION TO REMOVE HALF OF THE POPULATION OF TIME-TABLES THAT HAVE THE HIGHEST COST OF ERRORS ===================*/

void kill_costly_colony_members()
{
	int amount_to_kill;
	float kill_ratio = 0.5; 
	struct time_table *curr_ptr;
	struct time_table *clear_ptr;
	int i;
	
	amount_to_kill = solution_colony.population_size * kill_ratio;
	curr_ptr = solution_colony.first_time_table;
	for (i=0; i<(solution_colony.population_size - amount_to_kill)-1;i++)
	{
		curr_ptr = curr_ptr -> next;
	}
	solution_colony.last_time_table = curr_ptr;
	curr_ptr = curr_ptr -> next;
	solution_colony.last_time_table -> next = NULL;
	do 
	{
		clear_ptr = curr_ptr;
		curr_ptr = curr_ptr -> next;
		free(clear_ptr);
		solution_colony.population_size--;
	}while (curr_ptr != NULL);
}

/*===================== FUNCTION TO ADD NEW RANDOM INFORMATION IN THE TIME-TABLES DURING BREEDING ===================*/

void mutate(struct time_table *curr_ptr)
{
	int temporary;
	int curr_room;
	int curr_day;
	int curr_hour;
	int random_room;
	int random_day;
	int random_hour;
	
	for(curr_room=0;curr_room<NUMBER_OF_ROOMS;curr_room++)
	{
		for(curr_day=0;curr_day<DAYS_IN_WEEK;curr_day++)
		{
			for(curr_hour=0;curr_hour<HOURS_IN_DAY;curr_hour++)
			{
				if((random()%1000)<mutation_rate) 
				{   
					temporary = curr_ptr -> table[curr_room][curr_day][curr_hour];
					random_room = random() % NUMBER_OF_ROOMS;
					random_day = random() % DAYS_IN_WEEK;
					random_hour = random() % HOURS_IN_DAY;
					curr_ptr -> table[curr_room][curr_day][curr_hour] = curr_ptr -> table[random_room][random_day][random_hour];
					curr_ptr -> table[random_room][random_day][random_hour] = temporary;
				}
			}
		}
	}
}

/*===================== FUNCTION THAT COMBINES A FEW PROPERTIES OF TWO TIME-TABLES TO MAKE A NEW TIME-TABLE ===================*/
 
void breed_colony()
{
	int cross_over_rate = 2;
	struct time_table *mother;
	struct time_table *father;
	struct time_table *test_ptr;
	struct time_table *curr_ptr;
	int i;
	int room;
	int day;
	int hour;
	int state=0;

	mother = solution_colony.first_time_table;
	father = mother -> next;
	
	if((test_ptr = (struct time_table*)malloc(sizeof(struct time_table))) == NULL) 
	{
		printf("\nInsufficient memory for Time Table\n");
		exit(0);
	}
	state = (random()%2)+1;
	for(day=0;day<DAYS_IN_WEEK;day++) 
	{
		for(hour=0;hour<HOURS_IN_DAY;hour++) 
		{
			for(room=0;room<NUMBER_OF_ROOMS;room++)
			{
				if (state==1) 
				{
					test_ptr -> table[room][day][hour] = mother -> table[room][day][hour];
					if ((random()%cross_over_rate)==0) 
					{
						state = 2;
					}
				} 
				else 
				{
					test_ptr -> table[room][day][hour] = father -> table[room][day][hour];
					if ((random()%cross_over_rate)==0) 
					{
						state = 1;
					}
				}
			}
		}
	}
	
	mutate(test_ptr);
	repair_0(test_ptr);
	repair_1(test_ptr);
	calculate_cost(test_ptr);
	
	curr_ptr = solution_colony.first_time_table;
	if (curr_ptr -> cost >= test_ptr -> cost) 
	{
		test_ptr -> next = curr_ptr;
		solution_colony.first_time_table = test_ptr;
		solution_colony.population_size++;
	} 
	else 
	{
		curr_ptr = solution_colony.first_time_table;
		while ((curr_ptr -> next != NULL) && ((curr_ptr -> next) -> cost <= test_ptr -> cost))
		{
				curr_ptr = curr_ptr -> next;
		}
		if (curr_ptr == solution_colony.last_time_table) 
		{
			solution_colony.last_time_table = test_ptr;
			test_ptr -> next = NULL;
		} 
		else 
		{
			test_ptr -> next = curr_ptr -> next;
		}
		curr_ptr -> next = test_ptr;
		solution_colony.population_size++;
	}
}



/*************************************************************************************/
/*************************************************************************************/
/*********************    MAIN FUNCTION STARTS HERE    *******************************/ 
/*************************************************************************************/
/*************************************************************************************/

void main()
{
	int num_of_time_tables;
	
	population_size = MAX_POPULATION_SIZE;
	
	//srand(3);
	
	parse_file();
	make_faculty_structure();
	make_course_structure();
	make_room_structure();
	find_related_classes();

	initialise_colony();
	find_average_cost();
	
	num_of_time_tables = MAX_POPULATION_SIZE;
	
	printf("\nTT1: %d  TT2: %d  TT3: %d  Average cost = %d\n",solution_colony.first_time_table -> cost, solution_colony.first_time_table -> next -> cost,solution_colony.last_time_table -> cost,solution_colony.average_cost);
	
	while(solution_colony.first_time_table -> cost > MAXIMUM_ALLOWED_COST)
	{
		kill_costly_colony_members();	
		while(solution_colony.population_size < MAX_POPULATION_SIZE)
		{
			breed_colony();
			num_of_time_tables = num_of_time_tables + (MAX_POPULATION_SIZE / 2);
		}
		find_average_cost();
		
		printf("\nTT1: %d  TT2: %d  TT3: %d  Average cost = %d\n",solution_colony.first_time_table -> cost, solution_colony.first_time_table -> next -> cost,solution_colony.last_time_table -> cost,solution_colony.average_cost);
	}
	
	printf("\nNumber of time tables generated = %d\n\n",num_of_time_tables);
	
	FILE *fp;
	if((fp = fopen(OUTPUT_FILE,"w"))==NULL)
	{
		printf("\nUnable to open output file\n");
		exit(0);
	}
	
	int i,n,course,day,hour,room,room1,day1,hour1,flag=0;
	char hour_arr[HOURS_IN_DAY];
	char day_arr[6][4];
	
	strcpy(day_arr[0],"MON");
	strcpy(day_arr[1],"TUE");
	strcpy(day_arr[2],"WED");
	strcpy(day_arr[3],"THU");
	strcpy(day_arr[4],"FRI");
	strcpy(day_arr[5],"SAT");
	
	for(i=0;i<HOURS_IN_DAY;i++)
	{
		hour_arr[i]='A'+i;
	}
	
	for(i=0;i<=106;i++)
	{
		printf("-");
		fprintf(fp,"-");
	}
	
	printf("\n| %-4s | %-15s | %-60s | %-4s | %-8s |\n","S.NO","COURSE ID","COURSE NAME AND FACULTY NAME","ROOM","SCHEDULE");
	fprintf(fp,"\n| %-4s | %-15s | %-60s | %-4s | %-8s |\n","S.NO","COURSE ID","COURSE NAME AND FACULTY NAME","ROOM","SCHEDULE");
	
	for(i=0;i<=106;i++)
	{
		printf("-");
		fprintf(fp,"-");
	}
	
	printf("\n");
	fprintf(fp,"\n");
	
	for(course=0;course<total_no_of_courses;course++)
	{
		for(day=0;day<DAYS_IN_WEEK;day++)
		{
			for(hour=0;hour<HOURS_IN_DAY;hour++)
			{
				for(room=0;room<NUMBER_OF_ROOMS;room++)
				{
					if(solution_colony.first_time_table->table[room][day][hour]==course)
					{
						if(flag==0)
						{
							flag=1;
							room1=room;
							day1=day;
							hour1=hour;
						}
						else
						{
							flag=2;
							break;
						}
					}
				}
				if(flag==2)
				{
					break;
				}
			}
			if(flag==2)
			{
				flag=0;
				break;
			}
		}
		printf("| %-4d | %-15s | %-60s | %-4d | %-3s(%c)   |\n",course+1,course_arr[course].course_id,course_arr[course].course_name,room_arr[room1].room_number,day_arr[day1],hour_arr[hour1]);
		printf("| %-4s | %-15s | %-60s | %-4d | %-3s(%c)   |\n","","",faculty_arr[course_arr[course].faculty_reference].faculty_name,room_arr[room].room_number,day_arr[day],hour_arr[hour]);
		fprintf(fp,"| %-4d | %-15s | %-60s | %-4d | %-3s(%c)   |\n",course+1,course_arr[course].course_id,course_arr[course].course_name,room_arr[room1].room_number,day_arr[day1],hour_arr[hour1]);
		fprintf(fp,"| %-4s | %-15s | %-60s | %-4d | %-3s(%c)   |\n","","",faculty_arr[course_arr[course].faculty_reference].faculty_name,room_arr[room].room_number,day_arr[day],hour_arr[hour]);
		
		for(i=0;i<=106;i++)
		{
			printf("-");
			fprintf(fp,"-");
		}
		printf("\n");
		fprintf(fp,"\n");
	}
	
	printf("\n\n\n");
	fprintf(fp,"\n\n\n");
	for(i=0;i<=112;i++)
	{
		printf("-");
		fprintf(fp,"-");
	}
	printf("\n| %-1s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s |\n","","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY");
	fprintf(fp,"\n| %-1s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s |\n","","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY");
	for(i=0;i<=112;i++)
	{
		printf("-");
		fprintf(fp,"-");
	}
	printf("\n");
	fprintf(fp,"\n");
	
	for(hour=0;hour<HOURS_IN_DAY;hour++)
	{
		for(room=0;room<NUMBER_OF_ROOMS;room++)
		{
			for(day=0;day<DAYS_IN_WEEK;day++)
			{
				if(day==0)
				{
					if(room==2)
					{
						printf("| %-1c |",hour_arr[hour]);
						fprintf(fp,"| %-1c |",hour_arr[hour]);
					}
					else
					{
						printf("| %-1s |","");
						fprintf(fp,"| %-1s |","");
					}
				}
				curr_arr[day].curr_room=room;
				curr_arr[day].curr_class=solution_colony.first_time_table->table[room][day][hour];
				if(solution_colony.first_time_table->table[room][day][hour]!=-1)
				{
					printf(" %-15s |",course_arr[solution_colony.first_time_table->table[room][day][hour]].course_id);
					fprintf(fp," %-15s |",course_arr[solution_colony.first_time_table->table[room][day][hour]].course_id);
				}
				else
				{
					printf(" %-15s |","");
					fprintf(fp," %-15s |","");
				}
			}
			printf("\n");
			fprintf(fp,"\n");
			for(day=0;day<DAYS_IN_WEEK;day++)
			{	
				if(day==0)
				{
					printf("| %-1s |","");
					fprintf(fp,"| %-1s |","");
				}
				if(solution_colony.first_time_table->table[room][day][hour]!=-1)
				{
					printf(" (%-4d)@%-8d |",course_arr[curr_arr[day].curr_class].faculty_id,room_arr[curr_arr[day].curr_room].room_number);
					fprintf(fp," (%-4d)@%-8d |",course_arr[curr_arr[day].curr_class].faculty_id,room_arr[curr_arr[day].curr_room].room_number);
				}
				else
				{
					printf(" %-15s |","");
					fprintf(fp," %-15s |","");
				}
			}
			printf("\n");
			fprintf(fp,"\n");
		}
		for(i=0;i<=112;i++)
		{
			printf("-");
			fprintf(fp,"-");
		}
		printf("\n");
		fprintf(fp,"\n");
	}
	
	printf("\n\n\n");
	fprintf(fp,"\n\n\n");
	
	for(i=0;i<57;i++)
	{
		printf("-");
		fprintf(fp,"-");
	}
	
	printf("\n| %-10s | %-40s |\n","FACULTY ID","FACULTY NAME");
	fprintf(fp,"\n| %-10s | %-40s |\n","FACULTY ID","FACULTY NAME");
	
	for(i=0;i<57;i++)
	{
		printf("-");
		fprintf(fp,"-");
	}
	
	for(n=0;n<total_no_of_faculty;n++)
	{
		printf("\n| %-10d | %-40s |",faculty_arr[n].faculty_id,faculty_arr[n].faculty_name);
		fprintf(fp,"\n| %-10d | %-40s |",faculty_arr[n].faculty_id,faculty_arr[n].faculty_name);	
	}
	
	printf("\n");
	fprintf(fp,"\n");
	for(i=0;i<57;i++)
	{
		printf("-");
		fprintf(fp,"-");
	}
	printf("\n");
	fclose(fp);	
}
