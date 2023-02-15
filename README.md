# ARP-Assignment3
Solution to the third *Advanced and Robot Programming* (ARP) assignment.

Authors:
JT2-Wimir
- Tomasz Maciej Strzesak (5714359)
- Jan Mikołaj Drozd (5646665)

# ARP-Assignment3
Base repository for the **third ARP assignment**.

The code to design, develop, test and deploy is a modified version of Assignment 2, including
client/server features. We refer to this as "application". In the modified application, process B, shared memory and the second ncurses window are
unchanged. Process A includes two new features:
1. client connection toward a similar application running on a different machine in the network
2. server connection for a similar application running on a different machine in the network

Therefore the application, when launched, asks for one execution modality:
1. normal, as assignment 2
2. server: the application does not use the keyboard for input: it receives input from another
application (on a different machine) running in client mode
3. client: the application runs normally as assignment 2 and sends its keyboard input to another
application (on a different machine) running in server mode
When selecting modes 2 and 3 the application obviously asks address and port of the companion
application. To assure that any application is able to properly connect to any other application (implemented by
some other student/group), a communication protocol must be defined.
IP protocol: TCP
data: a byte stream of char, one per key pressed.
(flush data if necessary).






## Compiling and running
To compile the program:
```console
sudo ./compile.sh
```

## Executing 
```console
sudo ./run.sh
```

or

```console
sudo ./bin/master
```

## Repository link
```
gh repo clone drozdja/ARP-Assignment3
```

