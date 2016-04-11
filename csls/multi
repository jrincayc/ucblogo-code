PROGRAM MULTI(OUTPUT);
 {MULTINOMIAL EXPANSION PROBLEM}

VAR MEMO: ARRAY [0..4, 0..7] OF INTEGER;
    I,J:  INTEGER;

FUNCTION T(N,K:INTEGER) : INTEGER;

   FUNCTION REALT(N,K:INTEGER) : INTEGER;
    {WITHOUT MEMOIZATION}

      BEGIN {REALT}
         IF K = 0 THEN
            REALT := 1
         ELSE
            IF N = 0 THEN
               REALT := 0
            ELSE
               REALT := T(N,K-1)+T(N-1,K)
      END; {REALT}

   BEGIN {T}
      IF MEMO[N,K] < 0 THEN
         MEMO[N,K] := REALT(N,K);
      T := MEMO[N,K]
   END; {T}

BEGIN {MAIN PROGRAM}
   {INITIALIZATION}
   FOR I := 0 TO 4 DO
      FOR J := 0 TO 7 DO
         MEMO[I,J] := -1;

   {HOW MANY TERMS IN (A+B+C+D)^7}
   WRITELN(T(4,7));
END.
