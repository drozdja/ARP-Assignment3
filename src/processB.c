#include "./../include/processB_utilities.h"
#include "./../include/common.h"

// Utility variable to avoid trigger resize event on launch
int first_resize = TRUE;
int center_of_circle[2]; // declaring int array
FILE *fd_log;
char log_msg[200];

int logging(char *log)
{
    fd_log = fopen("out/processB.log", "a+");
    fprintf(fd_log, "Time: %ld; Msg: %s\n", time(NULL), log); // adding log msg to logfile
    fflush(fd_log);
    fclose(fd_log);
}

int create_shared_memory()
{                                                                         // function that creates shared memory
    int mem_size = WIDTH * HEIGHT * sizeof(rgb_pixel_t);                  // define shared memory size
    int share_mem = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, DEFFILEMODE); // open share memory
    if (share_mem < 0)                                                    // in case of failure
    {
        sprintf(log_msg, "Cannot open shared memory!");
        logging(log_msg);
        perror("Cannot open shared memory!");
        exit(1);
    }
    return share_mem;
}
rgb_pixel_t *create_matrix(int sh) // function that maps matrix
{
    rgb_pixel_t *matrix = (rgb_pixel_t *)mmap(NULL, (WIDTH * HEIGHT * sizeof(rgb_pixel_t)), PROT_READ | PROT_WRITE, MAP_SHARED, sh, 0); // mapping shared memory
    if (matrix == MAP_FAILED)                                                                                                           // in case of failure
    {
        sprintf(log_msg, "Cannot map!");
        logging(log_msg);
        perror("Cannot map!");
        exit(1);
    }

    return matrix;
}
int main()
{
    sleep(1);
    sprintf(log_msg, "ProcessB started");
        logging(log_msg);
    // Initialize UI
    init_console_ui();
    sem_t *semaphor = sem_open(SEM_PATH, 1); // opening semaphore
    if (semaphor == SEM_FAILED)              // in case of failure
    {
        sprintf(log_msg, "Cannot open semaphore!");
        logging(log_msg);
        perror("Cannot open semaphore!");
        exit(1);
    }
    int sm = create_shared_memory();      // call a function
    rgb_pixel_t *mat = create_matrix(sm); // call a function
    while (TRUE)
    {

        int cmd = getch(); // getting input from keyboard

        if (cmd == KEY_RESIZE) // if widnow resize
        {
            if (first_resize)
            {
                first_resize = FALSE;
            }
            else
            {
                reset_console_ui();
            }
        }

        else // find center of a circle
        {
            find_center(mat, center_of_circle);                                       // calling function
            mvaddch(center_of_circle[1] / RADIUS, center_of_circle[0] / RADIUS, '0'); // show the center
            //sprintf(log_msg, "Created center of circle with coordinates: %d, %d", center_of_circle[1] / RADIUS, center_of_circle[0] / RADIUS);
            //logging(log_msg);
        }
    }
    // closing shared memory and unlinking semaphore
    close(sm);
    shm_unlink(SHMOBJ_PATH);
    endwin();
    sprintf(log_msg, "ProcessB terminated");
        logging(log_msg);
    return 0;
}