program pi;
var i, rand, ca, cm, pi, qpi, x, y;
begin
   i := 0;
   qpi := 0;
   rand := 3827;
   ca := 8358;
   cm := 3797;
   while i < 1000 do begin
      rand := (rand + ca) * cm - (((rand + ca) * cm) div 100000) * 100000;
      rand := rand div 10;
      x := rand div 10;
      rand := (rand + ca) * cm - (((rand + ca) * cm) div 100000) * 100000;
      rand := rand div 10;
      y := rand div 10;
      if x * x + y * y < 1000000 then
	 qpi := qpi + 1;
      i := i + 1
   end;
   pi := qpi * 4;
   write pi
end.