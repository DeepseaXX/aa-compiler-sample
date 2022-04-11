program ex25;
var i, j, d, q, r, t, n;
begin
   i := 1;
   n := 100;
   while i <= n do
   begin
      j := 1;
	  d := 0;
	  while j <= i do
	  begin
	     q := i div j;
		 t := q * j;
		 r := i - t;
		 if r = 0 then
		    d := d + 1
		 j := j + 1
	  end;
	  if d = 2 then
	     write i;
      i := i + 1
   end
end.
