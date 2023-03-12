/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1990, 2001 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Lawrence Berkeley Laboratory,
 * Berkeley, CA.  The name of the University may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */



#ifndef lint
static const char rcsid[] =
    "@(#) $Header: /mvalla/tcp-w-nr.cc,v 1.2 2001/09/17 15:12:29 mvalla Exp mvalla $ (LBL)";
#endif

//
// tcp-w-nr: a revised New Reno TCP source, with faster recovery
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#include "packet.h"
#include "ip.h"
#include "tcp.h"
#include "flags.h"
#include "address.h"

#include "tcp-westwood-nr.h"

static class WestwoodNRTcpClass : public TclClass {
public:
	WestwoodNRTcpClass() : TclClass("Agent/TCP/WestwoodNR") {}
	TclObject* create(int, const char*const*) {
		return (new WestwoodNRTcpAgent());
	}
} class_westwoodnr;

///// 
// WestwoodNRTcpAgent()
WestwoodNRTcpAgent::WestwoodNRTcpAgent() : NewRenoTcpAgent(),
  // these where originally in TcpAgent()
  current_bwe_(0), last_bwe_sample_(0), unaccounted_(0),
  fr_a_(0), min_rtt_estimate(100.0), rtt_archive(5.0), k_rounds(20), modified(0), myseqno_(1),last_ts_(0),last_echoed_ts_(0),last_seq_(0),
  lastackrx_(0.0), fr_alpha_(0.9), filter_type_(1), tau_(1.0), total_time_(0.0), total_size_(0.0), fr_prev_(20.0)

{
	// Read defaults variables from ns-defaults.tcl

	// these where originally in TcpAgent()
	
	bind("current_bwe_", &current_bwe_);
	bind("last_bwe_sample_", &last_bwe_sample_);
  	bind("unaccounted_", &unaccounted_);
  	bind("fr_a_", &fr_a_);
	bind("fr_amin_", &fr_amin_);
	bind("fr_amax_", &fr_amax_);
	bind("fr_prev_", &fr_prev_);
  	bind("min_rtt_estimate", &min_rtt_estimate);

	bind("fr_alpha_", &fr_alpha_);
	bind("filter_type_", &filter_type_);
	bind("tau_", &tau_);
	bind("west_type_",&west_type_);
	bind("qest_",&qest_);
	bind("total_time_",&total_time_);
	bind("total_size_",&total_size_);
	bind("interp_type_",&interp_type_);

	bind("last_ts_",&last_ts_);
	bind("last_echoed_ts_",&last_echoed_ts_);
	bind("last_seq_",&last_seq_);
	bind("last_cwnd_",&last_cwnd_);
	bind("current_ts_",&current_ts_);
	bind("current_echoed_ts_",&current_echoed_ts_);

	// these where originally in NewRenoTcpAgent()
	bind("newreno_changes_", &newreno_changes_);
	bind("newreno_changes1_", &newreno_changes1_);
	bind("exit_recovery_fix_", &exit_recovery_fix_);
	bind("partial_window_deflation_", &partial_window_deflation_);
	bind("openadd_", &openadd_);
	//printf("Westwood New Reno binding done!\n");

	//! Ababil
	// bind("rtt_archive", &rtt_archive);
	// bind("k_rounds", &k_rounds);
	bind("modified", &modified);
}

///// 
// dupack_action()
void WestwoodNRTcpAgent::dupack_action()
{
	int recovered = (highest_ack_ > recover_);
        int allowFastRetransmit = allow_fast_retransmit(last_cwnd_action_);
        if (recovered || (!bug_fix_ && !ecn_) || allowFastRetransmit) {
                goto reno_action;
        }

        if (ecn_ && last_cwnd_action_ == CWND_ACTION_ECN) {
                last_cwnd_action_ = CWND_ACTION_DUPACK;
                /*
                 * What if there is a DUPACK action followed closely by ECN
                 * followed closely by a DUPACK action?
                 * The optimal thing to do would be to remember all
                 * congestion actions from the most recent window
                 * of data.  Otherwise "bugfix" might not prevent
                 * all unnecessary Fast Retransmits.
                 */
                reset_rtx_timer(1,0);
                output(last_ack_ + 1, TCP_REASON_DUPACK);
                return;
        }

        if (bug_fix_) {
                /*
                 * The line below, for "bug_fix_" true, avoids
                 * problems with multiple fast retransmits in one
                 * window of data.
                 */
                return;
        }

reno_action:

/*    
     if (ssthresh_ > cwnd_) {
	fr_a_+=0.25;
	if (fr_a_ > 4)
	  fr_a_=4;
      } else {
	fr_a_ = 1;
      }
  	ssthresh_ = (int)((current_bwe_/size_/8) * min_rtt_estimate);
      	if (cwnd_ > ssthresh_) {
      		cwnd_ = ssthresh_;
      	}
	*/

double fr_now = Scheduler::instance().clock();
double rtt_estimate = t_rtt_ * tcp_tick_;

if ((rtt_estimate < min_rtt_estimate)&&(rtt_estimate > 0)) {
		   min_rtt_estimate = rtt_estimate;
		}


/* west_type_ = 3 west+
   */



//if (west_type_<=4)fr_a_=-1;

double sstemp=(((current_bwe_*(min_rtt_estimate))/((double)(size_*8.0))));

if (sstemp < 2) sstemp = 2;
//if (sstemp1 < 2) sstemp1 = 2;


		ssthresh_ = (int)(sstemp);

		if (cwnd_ > sstemp) {cwnd_ = sstemp;}



	trace_event("TCPWNR_FAST_RETX");
        recover_ = maxseq_;
        last_cwnd_action_ = CWND_ACTION_DUPACK;
        // The slowdown was already performed
        // slowdown(CLOSE_SSTHRESH_HALF|CLOSE_CWND_HALF);
        reset_rtx_timer(1,0);
        output(last_ack_ + 1, TCP_REASON_DUPACK);
        return;

}

/////
// timeout()
void WestwoodNRTcpAgent::timeout(int tno)
{

	/* retransmit timer */
	if (tno == TCP_TIMER_RTX) {
		if (highest_ack_ == maxseq_ && !slow_start_restart_) {
			/*
			 * TCP option:
			 * If no outstanding data, then don't do anything.
			 */
			return;
		};
		recover_ = maxseq_;
		if (highest_ack_ == -1 && wnd_init_option_ == 2)
			/* 
			 * First packet dropped, so don't use larger
			 * initial windows. 
			 */
			wnd_init_option_ = 1;
		if (highest_ack_ == maxseq_ && restart_bugfix_)
		       /* 
			* if there is no outstanding data, don't cut 
			* down ssthresh_.
			*/
			slowdown(CLOSE_CWND_ONE);
		else if (highest_ack_ < recover_ &&
		  last_cwnd_action_ == CWND_ACTION_ECN) {
		       /*
			* if we are in recovery from a recent ECN,
			* don't cut down ssthresh_.
			*/
			slowdown(CLOSE_CWND_ONE);
		}
		else {
			++nrexmit_;
			slowdown(CLOSE_FASTER);
		}
		/* if there is no outstanding data, don't back off rtx timer */
		if (highest_ack_ == maxseq_ && restart_bugfix_) {
			reset_rtx_timer(0,0);
		}
		else {
			reset_rtx_timer(0,1);
		}
		last_cwnd_action_ = CWND_ACTION_TIMEOUT;
		send_much(0, TCP_REASON_TIMEOUT, maxburst_);

	} 
	else {
		timeout_nonrtx(tno);
	}
}

///// 
// bwe_computation()
void WestwoodNRTcpAgent::bwe_computation(Packet *pkt) {
	
	hdr_tcp *tcph = hdr_tcp::access(pkt);
	double fr_now = Scheduler::instance().clock();
	hdr_flags *fh = hdr_flags::access(pkt);
	
	// last_ack_ indicates the ack no. of the ack received _before_
	// the current one 
	
	// START BWE COMPUTATION
  // Idea: cumulative ACKs acking more than 2 packets count for 1 packet
	//   since DUPACKs have already been accounted for
	int cumul_ack = tcph->seqno_ - last_ack_;
	int cumul_ack1 = cumul_ack; //used for queueing time estimation
	myseqno_ = tcph->seqno_;

	if (cumul_ack > 1) {

	  /* check if current ACK ACKs fewer or same number of segments than */
	  /* expected: if so, the missing ones were already accounted for by */
	  /* DUPACKs, and current ACK only counts as 1 */
	  if (unaccounted_ >= cumul_ack) {
	    unaccounted_-=cumul_ack;
	    cumul_ack=1;
	  } else
	  /* check if current ACK ACKs more segments than expected: if so,   */
	  /* part of them were already accounted for by DUPACKs; the rest    */

	  /* are cumulatively ACKed by present ACK. Make present ACK count   */
	  /* as the unacknowledged ACKs in excess*/
	  if (unaccounted_ < cumul_ack) {
	    cumul_ack-=unaccounted_;
	    unaccounted_=0;
	  }
	}

  /* if cumul_ack=0, the current ACK is clearly a DUPACK and should */
	/* count 1 */
	if (cumul_ack == 0) {
	  unaccounted_++;
	  cumul_ack=1;
	}

  /* safety check; if the previous steps are followed exactly,      */
	/* cumul_ack should not be >2 unless some strage events occur     */
	/* (e.g., an ACK is dropped on the way back and the following one */
	/* appears to ACK more than its due)                              */

	if (cumul_ack > 2) {
	  cumul_ack=2;
	  }



	nackpack_+= cumul_ack;
	last_seq_+=cumul_ack;
	//qest_=cwnd_-(current_bwe_*min_rtt_estimate)/(8.0*(double)size_);

	current_ts_=tcph->ts();
	current_echoed_ts_=tcph->ts_echo();

	double rtt_estimate = t_rtt_ * tcp_tick_;
	
	

	  if ((rtt_estimate < min_rtt_estimate)&&(rtt_estimate > 0)) {
		  min_rtt_estimate = rtt_estimate;
		qest_=0;
 		last_echoed_ts_=current_echoed_ts_;
 		last_ts_=current_ts_;
		}

		qest_=qest_+(current_ts_-last_ts_)-(current_echoed_ts_-last_echoed_ts_);
		last_echoed_ts_=current_echoed_ts_;
 		last_ts_=current_ts_;


//if (west_type_==5) qest_=cwnd_-(current_bwe_*min_rtt_estimate)/(8.0*(double)size_);



	int acked_size = size_ * 8 * cumul_ack;
	double ack_interv = fr_now - lastackrx_;
	double sample_bwe;
	double last_tmp_bwe;
	int idle_intervals;

	switch (filter_type_) {
	  case 0:
          //   original filter
	  sample_bwe = acked_size/ack_interv;
	  current_bwe_ = current_bwe_ * fr_alpha_ + sample_bwe * (1 - fr_alpha_);
	  last_bwe_sample_ = sample_bwe;
	  break;

	  case 1:
	  // filter type 1
	  sample_bwe = acked_size/ack_interv;
	  current_bwe_ = current_bwe_ * .9047 +
	               (sample_bwe + last_bwe_sample_) * .0476;
	last_bwe_sample_ = sample_bwe;
	  break;
	
	  case 2:
	  // filter type 2: 'lower' pass
	  sample_bwe = acked_size/ack_interv;
	  current_bwe_ = current_bwe_ * .93548 + 
	               (sample_bwe+last_bwe_sample_) * .03225;

	  last_bwe_sample_ = sample_bwe;
	  break;

	  case 3:
	  // filter type 3: time constant tau_

	  // compute how many intervals of length tau_/2 went by since we
	  // received the last ACK. For each tau_/2 interval without ACK, feed
          // a zero-bandwidth sample to the filter.

	  idle_intervals = (int)(ack_interv / tau_*2.0);
	  //	  printf("idle_intervals = %d (%f,%f)=%f\n", idle_intervals, ack_interv, tau_,
	  //				ack_interv / tau_);
	  //		printf("idle_intervals = %d, ratio= %f\n",idle_intervals, ack_interv / tau_);
	  
	  ack_interv -= tau_ /2.0 * idle_intervals;
	  
	  //if ( (ack_interv < 0.01) && (idle_intervals == 0) ){
	  //	printf("TCP-W error: (ack_interv < 0.01) && (idle_intervals == 0)\n");
	  //	printf("time=%lf, last_ack=%lf, ack_interv=%lf\n", fr_now, lastackrx_, ack_interv);
	  //	//exit(0);
	  //}	
	  
	  if ( (ack_interv < 0.01) && (idle_intervals > 0) ) {
	  // ack_interv was a multiple of tau_/2 or the remainder is too small (less than 10ms), so 
	  // we consider tau_ / 2 as the last interval
	  	ack_interv = tau_ / 2.0;
		idle_intervals -= 1; // we do not count the last tau_/2 interval
	  }
	  
	  sample_bwe = acked_size/ack_interv;
	  
	  if (idle_intervals > 0) { // feed the filter
	  	// printf("idle_intervals = %d\n", idle_intervals);
		for (int i=0; i<idle_intervals; i++) {
			current_bwe_ = current_bwe_ * 3.0 / 5.0  + last_bwe_sample_/5.0;
			last_bwe_sample_ = 0.0;
			//  printf("idle_interval: current_bwe=%f\n", current_bwe_);
		}
	  }
	  
	  last_tmp_bwe = current_bwe_; // we need it just for the printf...
	  current_bwe_ = current_bwe_ * (2.0*tau_-ack_interv) /
		           (2.0*tau_+ack_interv) +
		           ack_interv*(sample_bwe + last_bwe_sample_)/(2.0*tau_+ack_interv);

	last_bwe_sample_ = sample_bwe;

	  if (current_bwe_ < 0) {
			printf("TCP-W error: current_bwe_ < 0\n");
			printf("time: %f, last_tmp_bwe=%f\n", fr_now, last_tmp_bwe);
			printf("current_bwe_%f, ack_interv=%f, sample_bwe=%f, last_bwe_sample_=%f\n",
					current_bwe_, ack_interv, sample_bwe, last_bwe_sample_);
			exit(0);
	  }
	  break;

	  case 4:



	total_size_ = total_size_ + acked_size;
	total_time_ = total_time_ + ack_interv;

	if (((rtt_estimate >0)&&(total_time_>rtt_estimate))) {


        sample_bwe = total_size_/total_time_;

	double m=(sample_bwe-last_bwe_sample_)/pow(total_time_,interp_type_);

	double sample_bwe_new=sample_bwe;
	double sample_bwe_old=last_bwe_sample_;
	double ack_delta=ack_interv;


	int Num_cicli;
	if (total_time_ < tau_/4.0) {
		Num_cicli = 0;
		ack_interv=total_time_;
		}
		else {
		Num_cicli = (int)(floor(((4.0*total_time_)/tau_)));
		ack_interv = total_time_-0.25*tau_*((double)(Num_cicli));
		}


		int i1;
  	for (i1=0;i1<(Num_cicli); i1++) 	{

		//if (m<0) {
			//sample_bwe=sample_bwe_old+m*pow((i1*tau_/4.0),interp_type_);
		//	}

		current_bwe_ = current_bwe_*(7.0/9.0)+(sample_bwe+last_bwe_sample_)/9.0;
		last_bwe_sample_ = sample_bwe;
		}

		if (ack_interv>0) {
		current_bwe_ = current_bwe_ * (2.0*tau_-ack_interv)/(2.0*tau_+ack_interv) +
		ack_interv*(sample_bwe_new+last_bwe_sample_)/(2.0*tau_+ack_interv);
		last_bwe_sample_ = sample_bwe_new;
		}




	total_time_=0.0;
	total_size_=0.0;
	}
	break;

  case 5:



	total_size_ = total_size_ + acked_size;
	total_time_ = total_time_ + ack_interv;

	if (((rtt_estimate >0)&&(total_time_>rtt_estimate))) {


        sample_bwe = total_size_/total_time_;

	current_bwe_=current_bwe_ * fr_alpha_ + sample_bwe*(1.0 - fr_alpha_);

	last_bwe_sample_ = sample_bwe;

	total_time_=0;
	total_size_=0;
	}
		break;

  case 6:



	total_size_ = total_size_ + acked_size;
	total_time_ = total_time_ + ack_interv;

	if (((rtt_estimate >0)&&(total_time_>rtt_estimate))) {


        sample_bwe = total_size_/total_time_;
	double sample_bwe1;
	sample_bwe1=0.5*(sample_bwe+last_bwe_sample_);

	current_bwe_=current_bwe_*fr_alpha_ + sample_bwe1*(1.0 - fr_alpha_);

	last_bwe_sample_ = sample_bwe;

	total_time_=0;
	total_size_=0;
	}
	break;
case 7:



	total_size_ = total_size_ + acked_size;
	total_time_ = total_time_ + ack_interv;

	if (((rtt_estimate >0)&&(total_time_>rtt_estimate))) {


        sample_bwe = total_size_/total_time_;

	double m=(sample_bwe-last_bwe_sample_)/pow(total_time_,interp_type_);

	double sample_bwe_new=sample_bwe;
	double sample_bwe_old=last_bwe_sample_;
	double ack_delta=ack_interv;


	int Num_cicli;
	if (total_time_ < tau_/4.0) {
		Num_cicli = 0;
		ack_interv=total_time_;
		}
		else {
		Num_cicli = (int)(floor(((4.0*total_time_)/tau_)));
		ack_interv = total_time_-0.25*tau_*((double)(Num_cicli));
		}


		int i1;
  	for (i1=0;i1<(Num_cicli); i1++) 	{


		sample_bwe=sample_bwe_old+m*pow((i1*tau_/4.0),interp_type_);

		current_bwe_ = current_bwe_*(7.0/9.0)+2.0*(sample_bwe)/9.0;
		last_bwe_sample_ = sample_bwe;
		}

		if (ack_interv>0) {
		current_bwe_ = current_bwe_ * (2.0*tau_-ack_interv)/(2.0*tau_+ack_interv) +
		2.0*ack_interv*(sample_bwe_new)/(2.0*tau_+ack_interv);
		last_bwe_sample_ = sample_bwe_new;
		}




	total_time_=0;
	total_size_=0;
	}

break;

  case 8:



	sample_bwe = acked_size/ack_interv;

	fr_alpha_=exp((-1.0*ack_interv/tau_));
	current_bwe_=current_bwe_*fr_alpha_ + sample_bwe*(1.0 - fr_alpha_);

	last_bwe_sample_ = sample_bwe;


break;

  case 9:



	total_size_ = total_size_ + acked_size;
	total_time_ = total_time_ + ack_interv;

	if (((rtt_estimate >0)&&(total_time_>rtt_estimate))) {


        sample_bwe = total_size_/total_time_;
	fr_alpha_=exp((-1.0*total_time_/tau_));
	current_bwe_=current_bwe_*fr_alpha_ + sample_bwe*(1.0 - fr_alpha_);

	last_bwe_sample_ = sample_bwe;

	total_time_=0;
	total_size_=0;
	}
	break;

case 10:



	total_size_ = total_size_ + acked_size;
	total_time_ = total_time_ + ack_interv;

	double ci;

	if (((rtt_estimate >0)&&(total_time_>rtt_estimate))) {

	//ci=cwnd_-(current_bwe_*min_rtt_estimate)/(8.0*(double)size_);

	if (qest_ > fr_a_) sample_bwe = total_size_/total_time_;
		else sample_bwe = last_bwe_sample_;

	fr_alpha_=exp((-1.0*total_time_/tau_));
	current_bwe_=current_bwe_*fr_alpha_ + sample_bwe*(1.0 - fr_alpha_);
	last_bwe_sample_ = sample_bwe;
	total_time_=0;
	total_size_=0;
	}




} // end of filter_type switch




double sstemp=(((current_bwe_*(min_rtt_estimate))/((double)(size_*8.0))));
//double sstemp1=0.9*qest_+(((current_bwe_*(min_rtt_estimate))/((double)(size_*8.0))));



#ifdef MYDEBUG
	hdr_ip *iph = hdr_ip::access(pkt);  
  	char *src_portaddr = Address::instance().print_portaddr(iph->sport());
	printf("sc%s: ack. no. %d at time %f, bwe=%f, cwnd = %d, ssthresh_ = %d\n",
	      src_portaddr, tcph->seqno_, fr_now, current_bwe_/1000000,
	      (int)cwnd_, (int)ssthresh_);
	printf("sc%s: now = %f, acked_size = %d, rxdiff = %f, last_ack_ = %d\n",
	         src_portaddr, fr_now, acked_size, (fr_now - lastackrx_), last_ack_);
	printf("sc%s: unaccounted_ = %d, fr_a_= %f, min_rtt_estimate = %f\n", 
			     src_portaddr, unaccounted_, fr_a_, min_rtt_estimate);
#endif
#ifdef MYDEBUG_RTT
	double f = t_rtt_ * tcp_tick_;
	printf("source %s: %f cwnd=%d	      bwe=%f	  rtt=%f\n", 
	      src_portaddr, fr_now, (int)cwnd_, current_bwe_/1000000, f);     
#endif	
#ifdef MYREPORT	
	hdr_ip *iph = hdr_ip::access(pkt);  
	char *src_portaddr = Address::instance().print_portaddr(iph->src());
	printf("%s    %f      %d      %f      %d\n", 
	      src_portaddr, fr_now, (int)cwnd_, current_bwe_/1000000,
	      (int)ssthresh_);        
#endif		

	lastackrx_ = fr_now;
}


/////
// recv()
void WestwoodNRTcpAgent::recv(Packet *pkt, Handler* h)
{
	// START BWE COMPUTATION
	bwe_computation(pkt);
	//double cwndapp,sstreshapp;
	//cwndapp=cwnd_;
	//sstreshapp=ssthresh_;
	NewRenoTcpAgent::recv(pkt,h);
	/*if ((cwnd_>cwndapp)&&(cwndapp<sstreshapp))
	{
	cwnd_=cwnd_+openadd_; //a more aggressive slow start
	send_much(0, 0, maxburst_);
	}*/
}

/////////////////// Added by MV
// these where originally in TcpAgent()


//! modifications
void WestwoodNRTcpAgent::modifiedopencwnd()
{
	double increment;
	if (cwnd_ < ssthresh_ && k_rounds > 0) {
		/* slow-start (exponential) */
		cwnd_ += 1;
		// printf("modified: %d\n", modified);
		//! modifications
		if(modified){
			double rtt_estimate = t_rtt_ * tcp_tick_;
			rtt_archive = rtt_estimate;
			// printf("rtt_estimate=%f, t_rtt_=%d\n", rtt_estimate,(int) t_rtt_);
			k_rounds--;
		}
	}
//  else {
// 		/* linear */
// 		double f;
// 		switch (wnd_option_) {
// 		case 0:
// 			if (++count_ >= cwnd_) {
// 				count_ = 0;
// 				++cwnd_;
// 			}
// 			break;

// 		case 1:
// 			/* This is the standard algorithm. */
// 			increment = increase_num_ / cwnd_;
// 			if ((last_cwnd_action_ == 0 ||
// 			  last_cwnd_action_ == CWND_ACTION_TIMEOUT) 
// 			  && max_ssthresh_ > 0) {
// 				increment = limited_slow_start(cwnd_,
// 				  max_ssthresh_, increment);
// 			}
// 			cwnd_ += increment;
// 			break;

// 		case 2:
// 			/* These are window increase algorithms
// 			 * for experimental purposes only. */
// 			/* This is the Constant-Rate increase algorithm 
//                          *  from the 1991 paper by S. Floyd on "Connections  
// 			 *  with Multiple Congested Gateways". 
// 			 *  The window is increased by roughly 
// 			 *  wnd_const_*RTT^2 packets per round-trip time.  */
// 			f = (t_srtt_ >> T_SRTT_BITS) * tcp_tick_;
// 			f *= f;
// 			f *= wnd_const_;
// 			/* f = wnd_const_ * RTT^2 */
// 			f += fcnt_;
// 			if (f > cwnd_) {
// 				fcnt_ = 0;
// 				++cwnd_;
// 			} else
// 				fcnt_ = f;
// 			break;

// 		case 3:
// 			/* The window is increased by roughly 
// 			 *  awnd_^2 * wnd_const_ packets per RTT,
// 			 *  for awnd_ the average congestion window. */
// 			f = awnd_;
// 			f *= f;
// 			f *= wnd_const_;
// 			f += fcnt_;
// 			if (f > cwnd_) {
// 				fcnt_ = 0;
// 				++cwnd_;
// 			} else
// 				fcnt_ = f;
// 			break;

//                 case 4:
// 			/* The window is increased by roughly 
// 			 *  awnd_ * wnd_const_ packets per RTT,
// 			 *  for awnd_ the average congestion window. */
//                         f = awnd_;
//                         f *= wnd_const_;
//                         f += fcnt_;
//                         if (f > cwnd_) {
//                                 fcnt_ = 0;
//                                 ++cwnd_;
//                         } else
//                                 fcnt_ = f;
//                         break;
// 		case 5:
// 			/* The window is increased by roughly wnd_const_*RTT 
// 			 *  packets per round-trip time, as discussed in
// 			 *  the 1992 paper by S. Floyd on "On Traffic 
// 			 *  Phase Effects in Packet-Switched Gateways". */
//                         f = (t_srtt_ >> T_SRTT_BITS) * tcp_tick_;
//                         f *= wnd_const_;
//                         f += fcnt_;
//                         if (f > cwnd_) {
//                                 fcnt_ = 0;
//                                 ++cwnd_;
//                         } else
//                                 fcnt_ = f;
//                         break;
//                 case 6:
//                         /* binomial controls */ 
//                         cwnd_ += increase_num_ / (cwnd_*pow(cwnd_,k_parameter_));                
//                         break; 
//  		case 8: 
// 			/* high-speed TCP, RFC 3649 */
// 			increment = increase_param();
// 			if ((last_cwnd_action_ == 0 ||
// 			  last_cwnd_action_ == CWND_ACTION_TIMEOUT) 
// 			  && max_ssthresh_ > 0) {
// 				increment = limited_slow_start(cwnd_,
// 				  max_ssthresh_, increment);
// 			}
// 			cwnd_ += increment;
//                         break;
// 		default:
// #ifdef notdef
// 			/*XXX*/
// 			error("illegal window option %d", wnd_option_);
// #endif
// 			abort();
// 		}
// 	}
	// if maxcwnd_ is set (nonzero), make it the cwnd limit
	if (maxcwnd_ && (int(cwnd_) > maxcwnd_))
		cwnd_ = maxcwnd_;

	return;
}


/*
 * open up the congestion window
 */
void WestwoodNRTcpAgent::opencwnd()
{
	if (modified) {
		modifiedopencwnd();
		return;
	}

	double increment;
	if (cwnd_ < ssthresh_) {
		/* slow-start (exponential) */
		cwnd_ += 1;
	}  else {
		/* linear */
		double f;
		switch (wnd_option_) {
		case 0:
			if (++count_ >= cwnd_) {
				count_ = 0;
				++cwnd_;
			}
			break;

		case 1:
			/* This is the standard algorithm. */
			increment = increase_num_ / cwnd_;
			if ((last_cwnd_action_ == 0 ||
			  last_cwnd_action_ == CWND_ACTION_TIMEOUT) 
			  && max_ssthresh_ > 0) {
				increment = limited_slow_start(cwnd_,
				  max_ssthresh_, increment);
			}
			cwnd_ += increment;
			break;

		case 2:
			/* These are window increase algorithms
			 * for experimental purposes only. */
			/* This is the Constant-Rate increase algorithm 
                         *  from the 1991 paper by S. Floyd on "Connections  
			 *  with Multiple Congested Gateways". 
			 *  The window is increased by roughly 
			 *  wnd_const_*RTT^2 packets per round-trip time.  */
			f = (t_srtt_ >> T_SRTT_BITS) * tcp_tick_;
			f *= f;
			f *= wnd_const_;
			/* f = wnd_const_ * RTT^2 */
			f += fcnt_;
			if (f > cwnd_) {
				fcnt_ = 0;
				++cwnd_;
			} else
				fcnt_ = f;
			break;

		case 3:
			/* The window is increased by roughly 
			 *  awnd_^2 * wnd_const_ packets per RTT,
			 *  for awnd_ the average congestion window. */
			f = awnd_;
			f *= f;
			f *= wnd_const_;
			f += fcnt_;
			if (f > cwnd_) {
				fcnt_ = 0;
				++cwnd_;
			} else
				fcnt_ = f;
			break;

                case 4:
			/* The window is increased by roughly 
			 *  awnd_ * wnd_const_ packets per RTT,
			 *  for awnd_ the average congestion window. */
                        f = awnd_;
                        f *= wnd_const_;
                        f += fcnt_;
                        if (f > cwnd_) {
                                fcnt_ = 0;
                                ++cwnd_;
                        } else
                                fcnt_ = f;
                        break;
		case 5:
			/* The window is increased by roughly wnd_const_*RTT 
			 *  packets per round-trip time, as discussed in
			 *  the 1992 paper by S. Floyd on "On Traffic 
			 *  Phase Effects in Packet-Switched Gateways". */
                        f = (t_srtt_ >> T_SRTT_BITS) * tcp_tick_;
                        f *= wnd_const_;
                        f += fcnt_;
                        if (f > cwnd_) {
                                fcnt_ = 0;
                                ++cwnd_;
                        } else
                                fcnt_ = f;
                        break;
                case 6:
                        /* binomial controls */ 
                        cwnd_ += increase_num_ / (cwnd_*pow(cwnd_,k_parameter_));                
                        break; 
 		case 8: 
			/* high-speed TCP, RFC 3649 */
			increment = increase_param();
			if ((last_cwnd_action_ == 0 ||
			  last_cwnd_action_ == CWND_ACTION_TIMEOUT) 
			  && max_ssthresh_ > 0) {
				increment = limited_slow_start(cwnd_,
				  max_ssthresh_, increment);
			}
			cwnd_ += increment;
                        break;
		default:
#ifdef notdef
			/*XXX*/
			error("illegal window option %d", wnd_option_);
#endif
			abort();
		}
	}
	// if maxcwnd_ is set (nonzero), make it the cwnd limit
	if (maxcwnd_ && (int(cwnd_) > maxcwnd_))
		cwnd_ = maxcwnd_;

	return;
}

/////
void
WestwoodNRTcpAgent::modifiedslowdown(int how)
{
	double win, halfwin, decreasewin;
	//! Ababil
	// int krounds_ = 20;
	int slowstart = 0;
	double fr_now = Scheduler::instance().clock();
	// we are in slowstart for sure if cwnd < ssthresh
	if (cwnd_ < ssthresh_ && k_rounds > 0) {
		slowstart = 1;

		// k_rounds--;
	}
	// we are in slowstart - need to trace this event
	trace_event("SLOW_START");

    if (precision_reduce_) {
		halfwin = windowd() / 2;
                if (wnd_option_ == 6) {
                        /* binomial controls */
                        decreasewin = windowd() - (1.0-decrease_num_)*pow(windowd(),l_parameter_);
                } else
	 		decreasewin = decrease_num_ * windowd();
		win = windowd();
	} else  {
		int temp;
		temp = (int)(window() / 2);
		halfwin = (double) temp;
                if (wnd_option_ == 6) {
                        /* binomial controls */
                        temp = (int)(window() - (1.0-decrease_num_)*pow(window(),l_parameter_));
                } else
	 		temp = (int)(decrease_num_ * window());
		decreasewin = (double) temp;
		win = (double) window();
	}
	// if (how & CLOSE_SSTHRESH_HALF)
	// 	// For the first decrease, decrease by half
	// 	// even for non-standard values of decrease_num_.
	// 	if (first_decrease_ == 1 || slowstart ||
	// 		last_cwnd_action_ == CWND_ACTION_TIMEOUT) {
	// 		// Do we really want halfwin instead of decreasewin
	// 		// after a timeout?
	// 		ssthresh_ = (int) halfwin;
	// 	} else {
	// 		ssthresh_ = (int) decreasewin;
	// 	}
    // else if (how & THREE_QUARTER_SSTHRESH)
	// 	if (ssthresh_ < 3*cwnd_/4)
	// 		ssthresh_  = (int)(3*cwnd_/4);
	// if (how & CLOSE_CWND_HALF)
	// 	// For the first decrease, decrease by half
	// 	// even for non-standard values of decrease_num_.
	// 	if (first_decrease_ == 1 || slowstart || decrease_num_ == 0.5) {
	// 		cwnd_ = halfwin;
	// 	} else cwnd_ = decreasewin;
    //     else if (how & CWND_HALF_WITH_MIN) {
	// 	// We have not thought about how non-standard TCPs, with
	// 	// non-standard values of decrease_num_, should respond
	// 	// after quiescent periods.
    //             cwnd_ = decreasewin;
    //             if (cwnd_ < 1)
    //                     cwnd_ = 1;
	// }
	// // ///
	// else if (how & CLOSE_FASTER) {
    // 	// TCP Westwood
	// 	// this might be critical what with the coarseness of the timer;
    // 	// keep in mind that TCP computes the timeout as
    // 	//              (#of ticks) * (tick_duration)
    // 	// We need to do away with the coarseness...


	double rtt_estimate = t_rtt_ * tcp_tick_;
	// printf("before min");
	if ((rtt_estimate <= min_rtt_estimate)&&(rtt_estimate > 0)) {
		min_rtt_estimate = rtt_estimate;
		// printf("in min\n");
	}
	

	double rtt_change = rtt_archive - rtt_estimate;
	double beta_ = 0.015;
	// printf("rtt_change: %f\n", rtt_change);
	// printf("rtt_archive: %f\n", rtt_archive);
	if ((rtt_archive == 0 || abs(rtt_change)/rtt_archive) > beta_) {
		cwnd_ = (((current_bwe_*(min_rtt_estimate))/((double)(size_*8.0))));
		if (cwnd_ < 1) cwnd_ = 1;
		rtt_archive = rtt_estimate;
		// printf("cwnd: %d\n", (int) cwnd_);
		// printf("rtt_archive after cwnd change: %f\n", rtt_archive);

	}




	// }


	// else if (how & CLOSE_CWND_RESTART)
	// 	cwnd_ = int(wnd_restart_);
	// else if (how & CLOSE_CWND_INIT)
	// 	cwnd_ = int(wnd_init_);
	// else if (how & CLOSE_CWND_ONE)
	// 	cwnd_ = 1;
	// else if (how & CLOSE_CWND_HALF_WAY) {
	// 	// cwnd_ = win - (win - W_used)/2 ;
	// 	cwnd_ = W_used + decrease_num_ * (win - W_used);
    //     if (cwnd_ < 1) cwnd_ = 1;
	// }
	if (ssthresh_ < 2)
		ssthresh_ = 2;
	if (how & (CLOSE_CWND_HALF|CLOSE_CWND_RESTART|CLOSE_CWND_INIT|CLOSE_CWND_ONE|CLOSE_CWND_HALF_WAY|CLOSE_FASTER))
		cong_action_ = TRUE;

	fcnt_ = count_ = 0;
	if (first_decrease_ == 1)
		first_decrease_ = 0;
}



// slowdown()
void
WestwoodNRTcpAgent::slowdown(int how)
{
	if(modified){
		modifiedslowdown(how);
		return;
	}

	double win, halfwin, decreasewin;
	int slowstart = 0;
	double fr_now = Scheduler::instance().clock();
	// we are in slowstart for sure if cwnd < ssthresh
	if (cwnd_ < ssthresh_)
		slowstart = 1;
	// we are in slowstart - need to trace this event
	trace_event("SLOW_START");

        if (precision_reduce_) {
		halfwin = windowd() / 2;
                if (wnd_option_ == 6) {
                        /* binomial controls */
                        decreasewin = windowd() - (1.0-decrease_num_)*pow(windowd(),l_parameter_);
                } else
	 		decreasewin = decrease_num_ * windowd();
		win = windowd();
	} else  {
		int temp;
		temp = (int)(window() / 2);
		halfwin = (double) temp;
                if (wnd_option_ == 6) {
                        /* binomial controls */
                        temp = (int)(window() - (1.0-decrease_num_)*pow(window(),l_parameter_));
                } else
	 		temp = (int)(decrease_num_ * window());
		decreasewin = (double) temp;
		win = (double) window();
	}
	if (how & CLOSE_SSTHRESH_HALF)
		// For the first decrease, decrease by half
		// even for non-standard values of decrease_num_.
		if (first_decrease_ == 1 || slowstart ||
			last_cwnd_action_ == CWND_ACTION_TIMEOUT) {
			// Do we really want halfwin instead of decreasewin
			// after a timeout?
			ssthresh_ = (int) halfwin;
		} else {
			ssthresh_ = (int) decreasewin;
		}
        else if (how & THREE_QUARTER_SSTHRESH)
		if (ssthresh_ < 3*cwnd_/4)
			ssthresh_  = (int)(3*cwnd_/4);
	if (how & CLOSE_CWND_HALF)
		// For the first decrease, decrease by half
		// even for non-standard values of decrease_num_.
		if (first_decrease_ == 1 || slowstart || decrease_num_ == 0.5) {
			cwnd_ = halfwin;
		} else cwnd_ = decreasewin;
        else if (how & CWND_HALF_WITH_MIN) {
		// We have not thought about how non-standard TCPs, with
		// non-standard values of decrease_num_, should respond
		// after quiescent periods.
                cwnd_ = decreasewin;
                if (cwnd_ < 1)
                        cwnd_ = 1;
	}
	///
	else if (how & CLOSE_FASTER) {
    	// TCP Westwood
	// this might be critical what with the coarseness of the timer;
    	// keep in mind that TCP computes the timeout as
    	//              (#of ticks) * (tick_duration)
    	// We need to do away with the coarseness...


	double rtt_estimate = t_rtt_ * tcp_tick_;

	  if ((rtt_estimate <= min_rtt_estimate)&&(rtt_estimate > 0)) {
		   min_rtt_estimate = rtt_estimate;
		}

	 double sstemp=(((current_bwe_*(min_rtt_estimate))/((double)(size_*8.0))));
			if (sstemp < 2) sstemp = 2;
			ssthresh_ = (int)(sstemp);
			cwnd_ = 2;

	}
//printf("set timeout = %f%f\n", fr_now,ssthresh_);


	else if (how & CLOSE_CWND_RESTART)
		cwnd_ = int(wnd_restart_);
	else if (how & CLOSE_CWND_INIT)
		cwnd_ = int(wnd_init_);
	else if (how & CLOSE_CWND_ONE)
		cwnd_ = 1;
	else if (how & CLOSE_CWND_HALF_WAY) {
		// cwnd_ = win - (win - W_used)/2 ;
		cwnd_ = W_used + decrease_num_ * (win - W_used);
                if (cwnd_ < 1)
                        cwnd_ = 1;
	}
	if (ssthresh_ < 2)
		ssthresh_ = 2;
	if (how & (CLOSE_CWND_HALF|CLOSE_CWND_RESTART|CLOSE_CWND_INIT|CLOSE_CWND_ONE))
		cong_action_ = TRUE;

	fcnt_ = count_ = 0;
	if (first_decrease_ == 1)
		first_decrease_ = 0;
}

/////
// newack()
/*
 * Process a packet that acks previously unacknowleged data.
 */


void WestwoodNRTcpAgent::newack(Packet* pkt)
{
	hdr_tcp *tcph = hdr_tcp::access(pkt);
	myseqno_ = tcph->seqno_;
	//call parent newack
	NewRenoTcpAgent::newack(pkt);
}

///// 
// delay_bind_dispatch()
//Westwood binds
int
WestwoodNRTcpAgent::delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer)
{

	if (delay_bind(varName, localName, "lastackno_", &lastackno_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "lastackrx_", &lastackrx_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "fr_alpha_", &fr_alpha_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "filter_type_", &filter_type_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "tau_", &tau_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "mss_", &mss_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "current_bwe_", &current_bwe_, tracer)) return TCL_OK;
       	if (delay_bind(varName, localName, "last_bwe_sample_", &last_bwe_sample_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "unaccounted_", &unaccounted_, tracer)) return TCL_OK;
        if (delay_bind(varName, localName, "fr_a_", &fr_a_, tracer)) return TCL_OK;
        if (delay_bind(varName, localName, "min_rtt_estimate", &min_rtt_estimate, tracer)) return TCL_OK;
  	if (delay_bind(varName, localName, "myseqno_", &myseqno_, tracer)) return TCL_OK;
	
	// these where originally in NewRenoTcpAgent()
	if (delay_bind(varName, localName, "newreno_changes_", &newreno_changes_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "newreno_changes1_", &newreno_changes1_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "exit_recovery_fix_", &exit_recovery_fix_, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "partial_window_deflation_", &partial_window_deflation_, tracer)) return TCL_OK;

        return NewRenoTcpAgent::delay_bind_dispatch(varName, localName, tracer);
}

/* tickoff is the time since the clock last ticked when
 *  the packet we are using to compute the RTT was sent
 */

/* t_rtt_ is the number of ticks that have occurred so far,
 * starting from the tick BEFORE the packet was sent
 */


