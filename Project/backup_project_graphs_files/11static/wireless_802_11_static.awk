BEGIN {
	max_node = 2000;
	nSentPackets = 0.0 ;		
	nReceivedPackets = 0.0 ;
	rTotalDelay = 0.0 ;
	max_pckt = 10000;
	
	idHighestPacket = 0;
	idLowestPacket = 100000;
	rStartTime = 10000.0;
	rEndTime = 0.0;
	nReceivedBytes = 0;

	nDropPackets = 0.0;

	total_energy_consumption = 0;

	temp = 0;
	
	for (i=0; i<max_node; i++) {
		energy_consumption[i] = 0;		
	}

	total_retransmit = 0;
	for (i=0; i<max_pckt; i++) {
		retransmit[i] = 0;		
	}
}

{

	strEvent = $1 ;			rTime = $2 ;
	node = $3 ;
	strAgt = $4 ;			idPacket = $6 ;
	strType = $7 ;			nBytes = $8;

	energy = $13;			total_energy = $14;
	idle_energy_consumption = $16;	sleep_energy_consumption = $18; 
	transmit_energy_consumption = $20;	receive_energy_consumption = $22; 
	num_retransmit = $30;
	
	sub(/^_*/, "", node);
	sub(/_*$/, "", node);

	if (energy == "[energy") {
		energy_consumption[node] = (idle_energy_consumption + sleep_energy_consumption + transmit_energy_consumption + receive_energy_consumption);
	}

	if( 0 && temp <=25 && energy == "[energy" && strEvent == "D") {
		temp+=1;
	}

	if ( strAgt == "AGT"   &&   strType == "tcp" ) {
		if (idPacket > idHighestPacket) idHighestPacket = idPacket;
		if (idPacket < idLowestPacket) idLowestPacket = idPacket;

		if(rTime>rEndTime) rEndTime=rTime;
		if(rTime<rStartTime) rStartTime=rTime;

		if ( strEvent == "s" ) {
			nSentPackets += 1 ;	rSentTime[ idPacket ] = rTime ;
		}
		if ( strEvent == "r" && idPacket >= idLowestPacket) {
			nReceivedPackets += 1 ;		nReceivedBytes += nBytes;
			rReceivedTime[ idPacket ] = rTime ;
			rDelay[idPacket] = rReceivedTime[ idPacket] - rSentTime[ idPacket ];
			rTotalDelay += rDelay[idPacket]; 

		}
	}

	if( strEvent == "D"   &&   strType == "tcp" )
	{
		if(rTime>rEndTime) rEndTime=rTime;
		if(rTime<rStartTime) rStartTime=rTime;
		nDropPackets += 1;
	}

	if( strType == "tcp" )
	{
		retransmit[idPacket] = num_retransmit;		
	}
}

END {
	rTime = rEndTime - rStartTime ;
	rThroughput = nReceivedBytes*8 / rTime;
	rPacketDeliveryRatio = nReceivedPackets / nSentPackets * 100 ;
	rPacketDropRatio = (nDropPackets) / nSentPackets * 100;

	for(i=0; i<max_node;i++) {
		total_energy_consumption += energy_consumption[i];
	}
	if ( nReceivedPackets != 0 ) {
		rAverageDelay = rTotalDelay / nReceivedPackets ;
		avg_energy_per_packet = total_energy_consumption / nReceivedPackets ;
	}

	if ( nReceivedBytes != 0 ) {
		avg_energy_per_byte = total_energy_consumption / nReceivedBytes ;
		avg_energy_per_bit = avg_energy_per_byte / 8;
	}

	for (i=0; i<max_pckt; i++) {
		total_retransmit += retransmit[i] ;		
	}


	print rThroughput, rAverageDelay, rPacketDeliveryRatio, rPacketDropRatio, avg_energy_per_packet;
}
