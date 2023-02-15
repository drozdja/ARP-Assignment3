#include "./../include/processA_utilities.h"
#include <bmpfile.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include "./../include/common.h"
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

//declaring variables
int shmfd;
char chared_data[10];
sem_t *sem;
rgb_pixel_t *ptr;
bmpfile_t *bmp;
FILE *fd_log;
char log_msg[200];
int mode;
int serv_fd;
int _socket; 
int port;
char buf[30] = {0};
char ip[15];
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int cmd;
int cmd2;
fd_set fd;
int val;
int x = (sizeof(client_addr));
const struct timespec t ={
  .tv_sec = 0,
  .tv_nsec = 30*1e6
};
int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return -1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) < 0) perror("Exec failed");
    return -1;
  }
}

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
        mode = atoi(argv[1]);
        if (argv[2][0] == '0'){
            port = 3000;}
        else 
        {sscanf(argv[2], "%d", &port);
        }
        sprintf(ip, "%s", argv[3]);
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
    if (mode > 1) { //if user chose server/client method
    
        if ((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            sprintf(log_msg, "Socket failed!");
            logging(log_msg);
            perror("Socket failed!");
        }
        bzero((char *)&server_addr, sizeof(server_addr));

        // Assigning port number and ip address
        server_addr.sin_port = htons(port);
        server_addr.sin_family = AF_INET;
        
        if (mode == 2) { //if user chose server mode
            if (ip[0] != '0') {
                server_addr.sin_addr.s_addr = htonl(INADDR_ANY);}
            else {
                server_addr.sin_addr.s_addr = inet_addr(ip);}
            
            if (bind(serv_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            {
                sprintf(log_msg, "Binding failed!");
            logging(log_msg);
            perror("Binding failed!");
            }

            // Listening
            if (listen(serv_fd, 2) < 0)
            {
                sprintf(log_msg, "Listening failed!");
            logging(log_msg);
            perror("Listening failed!");
            }

            // Accepting the connection
            if ((_socket = accept(serv_fd, (struct sockaddr *)& client_addr, (socklen_t *)&x)) < 0)
            {
                sprintf(log_msg, "Accepting failed!");
            logging(log_msg);
            perror("Accepting failed!");
            }

   
        } 
        if(mode==3) { //if user chose client mode
            server_addr.sin_addr.s_addr = inet_addr(ip);
        
            // Connecting to server
            if (connect(serv_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            {
                  sprintf(log_msg, "Connecting failed!");
            logging(log_msg);
            perror("Connecting failed!");
            }
        }
    }
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
       
        if (mode == 2) {
            
            FD_ZERO(&fd);
            FD_SET(_socket, &fd);

            val = select(_socket+1, &fd, NULL, NULL, 0);
            if (val < 0 && errno != EINTR) {
                sprintf(log_msg, "Error occured");
        logging(log_msg);
        perror("Error occured");
            }
            else if (val) {
                if (read(_socket, buf, sizeof(buf)) < 0) {
                    sprintf(log_msg, "Reading error");
                    logging(log_msg);
                    perror("Reading error");
                } else {
                    cmd = atoi(buf);}
            }
            cmd2 = getch();   
        } else {
            cmd = getch();
        } 

        if (mode == 3) { //client mode
            sprintf(buf, "%d", cmd);
            if (write(serv_fd, buf, sizeof(buf)) < 0) {
                 sprintf(log_msg, "Writing error");
                    logging(log_msg);
                    perror("Writing error");
            }
        }

        // If user resizes screen, re-draw UI...
        if (cmd == KEY_RESIZE || cmd2 == KEY_RESIZE)
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
        else if (cmd == KEY_MOUSE || cmd2 ==KEY_MOUSE)
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
      nanosleep(&t, NULL);
    }
    endwin();
    close_all(mem_size);
    sprintf(log_msg, "ProcessA terminated");
        logging(log_msg);
    return 0;
}
