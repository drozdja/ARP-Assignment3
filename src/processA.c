#include "./../include/processA_utilities.h"
#include "./../include/common.h"
//declaring variables
int shmfd;
char chared_data[10];
sem_t *sem;
rgb_pixel_t *ptr;
bmpfile_t *bmp;
FILE *fd_log;
char log_msg[200];

int logging(char *log)
{
    fd_log = fopen("out/processA.log", "a+");
    fprintf(fd_log, "%ld; processA; %s\n", time(NULL), log); // adding log msg to logfile
    fflush(fd_log);
    fclose(fd_log);
}

int init(int mem_size)
{
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH); //create bitmap
    if (bmp == NULL) //in case of failure
    {
        sprintf(log_msg, "Error creating bitmap");
        logging(log_msg);
        perror("Error creating bitmap");
        return -1;
    }

    int shmfd = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, DEFFILEMODE); //opening shared memory
    if (shmfd < 0)//in case of failure
    {
        sprintf(log_msg, "Error opening shared memory");
        logging(log_msg);
        perror("Error opening shared memory");
        return -1;
    }
    //
    int trunc = ftruncate(shmfd, mem_size); //configuring size of shared memory
    if (trunc < 0) //in case of failure
    {
        sprintf(log_msg, "Error configuring shared memory size");
        logging(log_msg);
        perror("Error configuring shared memory size");
        return -1;
    }

    // Shared memory mapping
    ptr = (rgb_pixel_t *)mmap(NULL, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (ptr == MAP_FAILED) //in case of failure
    {
        sprintf(log_msg, "Error mapping shared memory");
        logging(log_msg);
        perror("Error mapping shared memory");
        return -1;
    }

    // Opening semaphore
    sem = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
    {
        if (sem == SEM_FAILED) //in case of failure
        {
            sprintf(log_msg, "Error opening semaphore");
            logging(log_msg);
            perror("Error opening semaphore");
            return -1;
        }
    }
    return 0;
}

int close_all(int mem_size) //closingshared memory and semaphore
{
    shm_unlink(SHMOBJ_PATH);
    sem_close(sem);
    sem_unlink(SEM_PATH);
    munmap(ptr, mem_size);
}

int main(int argc, char *argv[])
{
    int mem_size = WIDTH * HEIGHT * sizeof(rgb_pixel_t); //shared memory size
    if (init(mem_size) != 0) //in case of failure
    {
        sprintf(log_msg, "Initialization failed");
        logging(log_msg);
        perror("Initialization failed");
        close_all(mem_size);
    }
    sprintf(log_msg, "ProcessA started");
        logging(log_msg);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();
    //functions for circle
    set_circle();
    draw_circle_bmp(bmp, circle.x/RADIUS, circle.y/RADIUS);
    save_bmp(bmp, ptr);
    
    if (sem_post(sem) < 0)
    {
        sprintf(log_msg, "Semaphore error");
        logging(log_msg);
        perror("Semaphore error");
    }

    // Infinite loop
    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if (cmd == KEY_RESIZE)
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

        // Else, if user presses print button...
        else if (cmd == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                if (check_button_pressed(print_btn, &event))
                {
                    bmp_save(bmp, "./out/snapshot.bmp"); //saving snapshot
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    sprintf(log_msg, "Print button pressed");
                    logging(log_msg);
                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if (cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN)
        {
            move_circle(cmd);
            draw_circle();

            // Reset bitmap
            bmp_destroy(bmp);
            bmp = bmp_create(WIDTH, HEIGHT, DEPTH);
            if (bmp == NULL)
            {
                sprintf(log_msg, "Error creating bitmap");
                logging(log_msg);
                perror("Error creating bitmap");
                break;
            }
            //functions for circle
            draw_circle_bmp(bmp, circle.x * RADIUS, circle.y * RADIUS);
            save_bmp(bmp, ptr);
            if (sem_post(sem) < 0)
            {
                sprintf(log_msg, "Semaphore error");
                logging(log_msg);
                perror("Semaphore error");
            }
        }
    }
    endwin();
    close_all(mem_size);
    sprintf(log_msg, "ProcessA terminated");
        logging(log_msg);
    return 0;
}