Replacement architecture for ~60k loc

System to allow writing of tests for most functionality of vmware esx server

Features yaml configuration files, colorful logs, encapsulation of functionality, 

There are two folders with very similar structure, the reason for that is so that
control can be exercized over what the user(developer) has access to, and otherwise
make things that aren't supposed to be accessed obviously difficult, this ironically
with the intent for better usability, so the user sees less code, although, in reality
everything should happen in the test file and the rest is framework background functionality.

Please ask me for any other details.

