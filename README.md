Readme file for megatron

First, the genschema file is to be executed, which generates the schema file in the same directory.

Then the actual megatron can be run.

Example Valid Queries:

select * from countries

select ID,NAME from countries

select distinct LATITUDE from airports

select * from countries where CONTINENT = AS

select ID,NAME from countries where CONTINENT = AS

select * from airports where ID > 370300 

select LATITUDE,LONGITUDE from airports where LATITUDE > LONGITUDE

select * from countries,airports where CODE = COUNTRY

select * from countries | abc


etc.
