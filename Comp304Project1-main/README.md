REPORT
maltinsoy19 - 71478
ckaplan19 - 71868
Filesearch
We started file search implementation with args manipulation which we are provided with. We divided file search into 2 parts. For the first basic file search there we write 2 words like “filesearch comp”. We checked dirent struct, opendir, readdir functions from stackoverflow and some other pages which provide us with method manuals. If the command name is equal to the name of the directory we are checking with dirent struct that is currently available and  we print the absolute path  of the directory for file search. For -r implementation we basically did the same thing but we did it recursively for the directories that are underneath it. However we used ListFiles method for this -r implementation. It takes two parameters and searches the strings we want to find in the directory names that are under our current working directory. We did some string manipulation to print the absolute paths there. For -o we used listFilesOpen method. This method is the similar to  listFiles but it opens the directories that it finds matching with the string. To open it in an application we used discussion board blackboard. And with your answer we managed to find what we wanted easily from stackoverflow to open the directories. 
Cdh
For cdh implementation we used cd command and modified it. Each time a directory is changed with cdh command we put it in an array. However with modulo operation we managed to show the most recently visited 10 directories every time when at least 10 directories are visited. We used chdir method to change into the last directory. Also in cdh command part we forked a child process and used scanf there. Also we used pipes to send the selected number from user. 

Take 

For take command we did basically file search to see if the directory  we wanted to open was already existing or not. If it was not there before we made it there and if it was the last directory like c in take a/b/ we changed directory there. We used mkdir, chdir methods and string manipulations to see make the command work. 
Joker 
For the joker command, we need to write on the crontab file our execution code as a whole. We first assemble by giving the crontab timing syntax, which is */15 * * * * for our code to execute every 15 min. Then we open the pre-formed file CR_joke.txt that we wrote the remaining code which includes, “XDG_RUNTIME_DIR=/run/user/$(id -u)” that opens our file inside crontab, “notify-send” that directs the joke to pop up on desktop, "$(/usr/bin/curl -s https://icanhazdadjoke.com/) that gets the joke from the url. 

Awesome Command 1 : Weather - Mert
Command gives the weather report of a specific requested location or gives the report based on the IP location of the user. Most of the code is included in the child process due to “scanf”. Results are brought by the “curl” command and “wttr.in” in the exec command. 

Awesome Command 1 : Personality Test - Celal
Personality test is a 10 question short answer test that determines if the user is an artistic or analytic person or an “all rounder” that means equal on both sides of the spectrum. It contains 10 scanf processes in the child process and the results are read in the parent process afterwards with the help of two pipes. 



Addition;
We did not see the last page of the document so we did not know that we were supposed to use github to show our progress. However, we know that it is our fault. I (Celal) asked you 2 questions on different days regarding two different questions from blackboard and Mert attended office hours to make some progress and get help from you. Honestly, we could not finish all parts of this homework successfully like kernel module. But we literally worked for hours even days to do  our best. We promise we will read the pdf carefully next time. Mert uploaded the last code on behalf of both of us.  



References
https://stackoverflow.com/  we almost used this site for every command and method.

http://www.computerhope.com/unix/ucrontab.htm 
http://manpages.ubuntu.com/manpages/xenial/man1/notify-send.1.html Used these links for the implementation of joker, specifically for gaining deeper understanding in Crontab and notify-send commands.

https://fishshell.com/docs/current/cmds/cdh.html learned about cdh command and its mechanics

https://www.tutorialspoint.com/cprogramming/index.htm got help from different sections of this site to learn more about several commands such as str variations like strcmp, strtok etc. 


https://www.geeksforgeeks.org/create-directoryfolder-cc-program/ used this to understand mkdir method. We used other pages from this website but It had been around 20 days so I can not remember each information we get from here. But website is quite useful for learning.

https://kremlin.cc/k&r.pdf  used this book to solve many problems we encountered during c implementations and sadly many segmentation faults. One of my favourite books. 


​​
https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html  to understand dirent struct
