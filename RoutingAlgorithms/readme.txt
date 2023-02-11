(i)The code was created in linux g++ environment 
(ii)The output file format is as follows 
   Number of Ports,Packet Generation Probability,Scheduling Policy,Average Packet Delay,
   Standard Deviation of Packet Delay,Average Link Utilization
(iii)To run the code - 
     (a)g++ routing.cpp -o routing
     ./routing -N 12 -B 5 -p 0.8 -queue KOUQ -K 10 -out out1.txt -T 10000 
     (b)The parameters that are be changed from their default values 
     must be specified along with the new values.
     (c)The parameters are -N(default value 8),-B(default value 4),-p(default value 0.5),
     -queue(default value INQ),-K(default value 5),-out(default value output.txt),-T(default value 10000)
     (d)If none of the values are to be changed from the default we can simply provide ./routing without any extra options
