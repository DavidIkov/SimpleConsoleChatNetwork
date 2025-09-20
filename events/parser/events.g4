grammar events;

structure: event*;
event: 'event' event_name '{' request respond '}';
event_name: NAME;
request: 'request' '{' field* '}';
respond: 'respond' '{' field* '}';
field: type NAME ';';
type: 'int32' | 'int64' | 'uint32' | 'uint64' | string | 'float' | 'bool' | enum_;

string: 'string' '(' str_len ')';
str_len: INT;

enum_: 'enum' '(' enum_name (',' enum_name)* ')';
enum_name: NAME;

INT: [0-9]+;
NAME: [a-zA-Z0-9_]+;
WS: [ \r\n]+ -> skip;
COMMENTS: '//'~[\r\n]+ -> skip;
