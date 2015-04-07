RESTORE @pic

# lines 22 and 23 would need to be printed in the #0 screen area
FOR A=0 TO 21
READ A$
PRINT AT A,0;A$;
NEXT A

@pic:
# paste data statements below this line
# and build with zmakebas -l


