PROGRAM PSORT(OUTPUT);
 {PARTITION SORT DEMO}

VAR DATA: ARRAY [-1..99] OF INTEGER;
    I: INTEGER;

PROCEDURE SHOWDATA;
 {PRINT THE ARRAY}

   VAR I: INTEGER;

   BEGIN {SHOWDATA}
      FOR I := 0 TO 99 DO
         BEGIN
            IF I MOD 20 = 0 THEN WRITELN;
            WRITE(DATA[I]:3)
         END;
      WRITELN;
      WRITELN
   END; {SHOWDATA}

PROCEDURE SORT(LOWER,UPPER:INTEGER);
 {SORT PART OF THE ARRAY}

   VAR KEY,I,J:INTEGER;

   PROCEDURE EXCH(VAR A,B:INTEGER);
    {EXCHANGE TWO INTEGERS}

      VAR TEMP:INTEGER;

      BEGIN {EXCH}
         TEMP := A;
         A := B;
         B := TEMP
      END; {EXCH}

   BEGIN {SORT}
      IF UPPER > LOWER THEN
         BEGIN
            I := LOWER;
            J := UPPER;
            KEY := (DATA[LOWER]+DATA[UPPER]) DIV 2;
            WHILE I < J DO
               BEGIN
                  WHILE DATA[I] < KEY DO
                     I := I+1;
                  IF I < J THEN
                     BEGIN
                        EXCH(DATA[I],DATA[J]);
                        REPEAT
                           J := J-1
                        UNTIL DATA[J] <= KEY
                     END;
                  IF I < J THEN
                     BEGIN
                        EXCH(DATA[I],DATA[J]);
                        I := I+1
                     END
               END;
            IF DATA[J] > KEY THEN J := J-1;
            SORT(LOWER,J);
            SORT(J+1,UPPER)
         END
   END; {SORT}

BEGIN {MAIN PROGRAM}
   DATA[-1] := -1;
   FOR I := 0 TO 99 DO
      DATA[I] := RANDOM(100);
   WRITELN('DATA BEFORE SORTING:');
   SHOWDATA;

   SORT(0,99);
   WRITELN('DATA AFTER SORTING:');
   SHOWDATA
END.
