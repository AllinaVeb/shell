Shell
=====
Program acting as Shell   

Installation 
-----
* create a directory "shell"   
* load in "shell" "bin", "source" and "Makefile"   
* open "shell" in terminal   
* enter `make`   
* for start enter `./bin/shell`   
* for finish enter `exit` or `quit`   


Command examples
----
* running one command   
`date`, `ls -o`, `sleep 5`, `pwd`, `rm somefile.txt`, ...   
* forwarding   
`ls > a.txt`, `ws < a.txt`, ...   
* conveyor &&(and) and ||(or)   
`ls && date && pwd`, `ls && wrong command or some trash && date`,  `wrong command or some trash || ls`, `date || pwd`, ...   
* conveyor |(pipe)   
`ls | sort |  wc`, ...   
* conveyor |(pipe) with forwarding   
`sort < a.txt | wc > b.txt`, ...   


