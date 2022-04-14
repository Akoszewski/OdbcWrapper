drop table if exists kontynenty;
create table kontynenty (id serial, nazwa varchar(20) not null);
insert into kontynenty (nazwa) values ('Azja');
insert into kontynenty (nazwa) values ('Europa');
insert into kontynenty (nazwa) values ('Ameryka');

select * from kontynenty;

