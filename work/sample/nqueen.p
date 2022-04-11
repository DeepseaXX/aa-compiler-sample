program nqueen;
var n, sum;

procedure putqueen(board, column, n);
var i, j, newboard, checkrow, tmpboard, mask, check; 
begin
   i := 1;
   while i <= n do
   begin
      tmpboard := board;
      mask := 0;
      j := column - 1;
      check := 0;
      while j > 0 do
      begin
         mask := tmpboard div 10;
         mask := mask * 10;
         checkrow := tmpboard - mask;
         if checkrow = i then
            check := 1
         else if checkrow = i + (column-j) then
            check := 1
         else if checkrow = i - (column-j) then
            check := 1;
         tmpboard := tmpboard div 10;
         j := j - 1
      end;
      if check = 0 then
      begin
         newboard := board * 10 + i;
         if column = n then
         begin
            write newboard;
            sum := sum + 1
         end
         else
            putqueen(newboard, column + 1, n)
      end;
      i := i + 1
   end
end;

begin
   n := 8;
   sum := 0;
   putqueen(0, 1, n);
   write sum
end.
