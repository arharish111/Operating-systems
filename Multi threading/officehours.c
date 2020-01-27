/*
 Name: Harish Harish
	
*/


#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */
#define consecutiveFive 5 

#define CLASSA 0
#define CLASSB 1

/* TODO */
/* Add your synchronization variables here */

/* Basic information about simulation.  They are printed/checked at the end 
 * and in assert statements during execution.
 *
 * You are responsible for maintaining the integrity of these variables in the 
 * code that you develop. 
 */
 
sem_t sem_enter_leave;  /*semphore for three seats */
sem_t sem_same_class;   /*semphore to make sure students are from same class */
sem_t sem_professor_break; /*semphore for professor's break */
sem_t sem_five;            /*semphore for five students from same class in a row*/
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER; /*mutex to guard pthread condition variable*/
pthread_mutex_t guard_post = PTHREAD_MUTEX_INITIALIZER; /*mutex to guard sem_post*/
pthread_mutex_t five_mutex = PTHREAD_MUTEX_INITIALIZER; /*mutex to guard count of five students from same class*/
pthread_cond_t  count_threshold_cv = PTHREAD_COND_INITIALIZER; /*mutex to guard count of students*/

static int countA;  /*Total classA students for five in a row check*/
static int countB;  /*Total classB students for five in a row check*/

static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students from class A currently in the office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break = 0;

typedef struct 
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
} student_info;

/* Called at beginning of simulation.  
 * TODO: Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename) 
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;

  /* Initialize your synchronization variables (and 
   * other variables you might use) here
   */

  sem_init(&sem_enter_leave,0,1);
  sem_init(&sem_same_class,0,1);
  sem_init(&sem_professor_break,0,1);
  sem_init(&sem_five,0,1);

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL) 
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d\n", &(si[i].arrival_time), &(si[i].question_time))!=EOF) && i < MAX_STUDENTS ) 
  {
    i++;
  }

 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break 
 * You do not need to add anything here.  
 */
static void take_break() 
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk) 
{
  printf("The professor arrived and is starting his office hours\n");
  /* Loop while waiting for students to arrive. */
  int i;
  while (1) 
  {

    /* TODO */
    /* Add code here to handle the student's request.             */
    /* Currently the body of the loop is empty. There's           */
    /* no communication between professor and students, i.e. all  */
    /* students are admitted without regard of the number         */ 
    /* of available seats, which class a student is in,           */
    /* and whether the professor needs a break. You need to add   */
    /* all of this.                                               */
	
	/*condition variable wait for professor's break               */
	/*sem_posting for two times for 20th student                  */
	if(students_since_break <= professor_LIMIT)
	{
		pthread_mutex_lock(&count_mutex);
		pthread_cond_wait(&count_threshold_cv,&count_mutex);
		take_break();
		printf("The professor is done taking a break.\n");
        for(i=0;i<=1;i++)
        {
	       sem_post(&sem_professor_break);
        }
		pthread_mutex_unlock(&count_mutex);
	}
 
  }
  pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter() 
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.                                                     */ 
  
  /* To make sure every requirement is satisfied by using semaphores,     */     
  /* pthread condition signal (professor break) and guarding              */
  /* by mutex lock and unlock                                             */
  
  pthread_mutex_lock(&five_mutex);
  countA+=1;
  if(countB>=1 && countB <=4)
	  countB=0;
  pthread_mutex_unlock(&five_mutex);
  if(countA>=consecutiveFive)
  {
	  sem_wait(&sem_five);
  }
  
  if(classa_inoffice == 0)
	  sem_wait(&sem_same_class);
  if(students_since_break == 10)
	  sem_wait(&sem_professor_break);
  if(classb_inoffice > 0)
  {
	  sem_wait(&sem_same_class);
  }
 
  if(students_in_office >= 2)
	  sem_wait(&sem_enter_leave);
  pthread_mutex_lock(&guard_post);
  students_in_office += 1;
  classa_inoffice += 1;
  pthread_mutex_unlock(&guard_post);
  pthread_mutex_lock(&count_mutex);
  students_since_break += 1;
  if(students_since_break == 10)
  {
	  sem_wait(&sem_professor_break);
  }
  pthread_mutex_unlock(&count_mutex);
}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter() 
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.                                                     */
  
  /* To make sure every requirement is satisfied by using sem_wait,       */     
  /* pthread condition signal (professor break) and guarding              */
  /* by mutex lock and unlock                                             */
  
  pthread_mutex_lock(&five_mutex);
  countB+=1;
  if(countA >=1 && countA <=4)
	  countA=0;
  pthread_mutex_unlock(&five_mutex);
  if(countB>=consecutiveFive)
  {
	  sem_wait(&sem_five);
  }
  
  
  if (classb_inoffice == 0)
      sem_wait(&sem_same_class);
  if(students_since_break == 10)
	  sem_wait(&sem_professor_break);
  if(classa_inoffice > 0)
  {
	  sem_wait(&sem_same_class);
  }
  
  if(students_in_office >= 2)
  {
	  sem_wait(&sem_enter_leave);
  }
	
  pthread_mutex_lock(&guard_post);
  students_in_office += 1;
  classb_inoffice += 1;
  pthread_mutex_unlock(&guard_post);
  pthread_mutex_lock(&count_mutex);
  students_since_break += 1;
  if(students_since_break == 10)
  {
	  sem_wait(&sem_professor_break);
  }	 
  pthread_mutex_unlock(&count_mutex);
}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.  
 */
static void ask_questions(int t) 
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classa_leave() 
{
  /* 
   *  TODO
   *  YOUR CODE HERE. 
   */

  students_in_office -= 1;
  classa_inoffice -= 1;
  int j;
  
  /* To make sure every requirement is satisfied by using sem_post,       */     
  /* guarding by mutex lock and unlock                                    */
  
  if(countB>=consecutiveFive)
  {
	  pthread_mutex_lock(&five_mutex);
	  countB = countB -5;
	  pthread_mutex_unlock(&five_mutex);
	  for(j=0;j<countB;j++)
	  {
	    sem_post(&sem_five);
	  }
  }
  
  
  pthread_mutex_lock(&guard_post);
  if(classa_inoffice==0)
	  sem_post(&sem_same_class);
  if(classa_inoffice >= 2)
      sem_post(&sem_enter_leave);
  pthread_mutex_unlock(&guard_post);
  pthread_mutex_lock(&count_mutex);
  if(students_in_office == 0 && students_since_break == 10)
      pthread_cond_signal(&count_threshold_cv);
  if(students_since_break == 0)
	  sem_post(&sem_professor_break);
  pthread_mutex_unlock(&count_mutex);

}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classb_leave() 
{
  /* 
   * TODO
   * YOUR CODE HERE. 
   */
  
  students_in_office -= 1;
  classb_inoffice -= 1;
  int i;
  
  /* To make sure every requirement is satisfied by using sem_post,       */     
  /* guarding by mutex lock and unlock                                    */
 
  if(countA>=consecutiveFive)
  {
	  pthread_mutex_lock(&five_mutex);
	  countA = countA -5;
	  pthread_mutex_unlock(&five_mutex);
	  for(i=0;i<countA;i++)
	  {
	    sem_post(&sem_five);
	  }
  }
  
  pthread_mutex_lock(&guard_post);
  if(classb_inoffice == 0)
	  sem_post(&sem_same_class);
  if(classb_inoffice >= 2)
      sem_post(&sem_enter_leave);
  pthread_mutex_unlock(&guard_post);
  pthread_mutex_lock(&count_mutex);
  if(students_in_office == 0 && students_since_break == 10)
  pthread_cond_signal(&count_threshold_cv);
  if(students_since_break == 0)
	  sem_post(&sem_professor_break);
  pthread_mutex_unlock(&count_mutex);

 
}

/* Main code for class A student threads.  
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter();

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );
  
  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave();  

  printf("Student %d from class A leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter();

  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);
  
  fflush(NULL);

  /* leave office */
  classb_leave();        

  printf("Student %d from class B leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 */
int main(int nargs, char **args) 
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2) 
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0) 
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result) 
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++) 
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);
                
    student_type = random() % 2;

    if (student_type == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result) 
    {
      printf("officehour: thread_fork failed for student %d: %s\n", 
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++) 
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
