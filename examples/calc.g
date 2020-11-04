
%INT=[0-9];
%NUMBER=INT+;
%STRING = [a-zA-Z]+;

%PLUS = `+`;
%SUB = `-`;
%MUL = `*`;
%DIV = `/`;

%expr = op PLUS op
    | op SUB op
    | op;

%op = op2 MUL op2
    | op2 DIV op2
    | op2;

%op2 = SUB INT | INT | SUB INT;
