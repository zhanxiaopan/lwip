/*
 *            Copyright (c) 2002-2013 by Real Time Automation, Inc.
 *
 *  This software is copyrighted by and is the sole property of
 *  Real Time Automation, Inc. (RTA).  All rights, title, ownership, 
 *  or other interests in the software remain the property of RTA.  
 *  This software may only be used in accordance with the corresponding
 *  license agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice MAY NOT be removed or modified without prior
 *  written consent of RTA.
 *
 *  RTA reserves the right to modify this software without notice.
 *
 *  Real Time Automation
 *  150 S. Sunny Slope Road            USA 262.439.4999
 *  Suite 130                          www.rtaautomation.com
 *  Brookfield, WI 53005               software@rtaautomation.com
 *
 *************************************************************************
 *
 *    Version Date: 05NOV2013
 *         Version: 2.37
 *    Conformed To: EtherNet/IP Protocol Conformance Test A-10 (28-AUG-2012)
 *     Module Name: eips_revision.h
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the stack revision history.
 *
 */

#ifndef __EIPS_REVISION_H__
#define __EIPS_REVISION_H__

#define EIPS_VERSION_STRING "2.37"
#define EIPS_MAJOR_REV       0x02
#define EIPS_MINOR_REV       0x25
#define EIPS_REVISION        0x2502 /* minor/major */

/* 
===========================================================================
11/05/2013  REVISION 02.37 (Author: Jamin D. Wendorf)   ** Conform to A-11 **
                                                       ** Nov 1 2013 Build **	
===========================================================================


===========================================================================
07/22/2013  REVISION 02.36 (Author: Jamin D. Wendorf)   ** Conform to A-10 **
                                                       ** Aug 28 2012 Build **	
===========================================================================
1.  Added "EIPS_USER_TCP_INACTIVITY_TMO" in eips_usersys.h to define the 
    number of ticks before an unused TCP connection is closed. The default
    is 0 which turns off the functionality.

===========================================================================
06/13/2013  REVISION 02.35 (Author: Jamin D. Wendorf)   ** Conform to A-10 **
                                                       ** Aug 28 2012 Build **	
===========================================================================
1. Modified eips_userobj.c/h to support Modbus Object (0x44).
2. Changed EIPS_USEROBJ_ASM_ELEMENT_SIZE to 2 and removed conditional 
    compiles in eips_userobj.c

===========================================================================
04/23/2013  REVISION 02.34 (Author: Jamin D. Wendorf)   ** Conform to A-10 **
                                                       ** Aug 28 2012 Build **	
===========================================================================
1.  Custom Point I/O Example for customer.

    The following is the Point I/O configuration:
    ----------      ----    --------------------------      ---------------------
    IP Address      Slot    Name                            Direct/Rack Optimized
    ----------      ----    --------------------------      ---------------------
    192.168.16.10	  0	    1734-AENT Ethernet Adapter      Head node
    192.168.16.10	  1	    1734-IV8 8 PT 24VDC SOURCE IN   Rack Optimized
    192.168.16.10  	  2	    1734-IB8 8 PT 24VDC SINK IN     Rack Optimized
    192.168.16.10	  3	    1734-OB8 8 PT 24VDC SOURCE OUT  Rack Optimized
    192.168.16.10	  4	    1734-IE8C 8 PT CURRENT INPUT    Direct
    192.168.16.10	  5	    1734-OE4C 4 PT CURRENT OUTPUT   Direct

===========================================================================
04/18/2013  REVISION 02.33 (Author: Jamin D. Wendorf)   ** Conform to A-10 **
                                                       ** Aug 28 2012 Build **	
===========================================================================
1.  Fixed Conformance Test errors (primarily in Register Session error handling).

===========================================================================
03/15/2013  REVISION 02.32 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1.  Re-added sample vendor specific object model.
2.  Added new reset function to perform a hardware reset (optionally) in
    eips_usersys.c. #define is in eips_usersys.h

    #ifdef EIPS_USERSYS_HWRESET_SUPPORTED
    void eips_usersys_noreturn_reboot (void);
    #endif

===========================================================================
01/05/2013  REVISION 02.31 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1. Added Tag Read/Write Fragmented to server/adapter.

===========================================================================
01/03/2013  REVISION 02.30 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1. Add support for NULL Class 1 ForwardOpen.

===========================================================================
12/20/2012  REVISION 02.29 (Author: Jamin D. Wendorf)
===========================================================================
1. Changed how client examples are processed. We now use a callback scheme.
2. Added UDT support to Tag Read/Write (normal and fragmented)
3. Added client examples to use new functionality. Only the example_logix*.c functions are tested.

===========================================================================
11/21/2012  REVISION 02.28 (Author: Jamin D. Wendorf)
===========================================================================
1. Added Tag Read/Write Fragmented to client/scanner.

===========================================================================
09/28/2012  REVISION 02.27 (Author: Jamin D. Wendorf)
===========================================================================
1. Changed eipc_io.c to allow a variable number of item on the FwdOpen response.

===========================================================================
08/10/2012  REVISION 02.26 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1. Added Network Who Broadcast to client code.

===========================================================================
07/27/2012  REVISION 02.25 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1. Fixed Tag Server.

===========================================================================
06/11/2012  REVISION 02.24 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1. Added Large Forward Open support

===========================================================================
05/13/2012  REVISION 02.23 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1. Update for Plugfest
2. Add ID Object: CCV and State; Expose Status to user code
3. Added better reset code 

===========================================================================
12/07/2011  REVISION 02.22 (Author: Jamin D. Wendorf)   ** Conform to A-9 **
                                                       ** Nov 16 2011 Build **	
===========================================================================
1.  Changed ForwardOpen error processing.
2.  Changed ForwardOpen Electronic Key parsing.
3.  Fixed Conformance Test errors and warnings.

===========================================================================
11/03/2011  REVISION 02.21 (Author: Jamin D. Wendorf)   ** Conform to A-8 **
            (started with 2.19)                         ** Oct 26 2010 Build **	
===========================================================================
1.  Added "EIPS_USER_TCP_INACTIVITY_TMO" in eips_usersys.h to define the 
    number of ticks before an unused TCP connection is closed. The default
    is 0 which turns off the functionality.
2.  Added "eips_usersock_close_all_sockets()" to close all TCP and UDP sockets.

===========================================================================
nn/nn/20nn  REVISION 02.20 (Author: Jamin D. Wendorf)   
===========================================================================
1.  Different beta code. Revision skipped. 

===========================================================================
11/10/2010  REVISION 02.19 (Author: Jamin D. Wendorf)   ** Conform to A-8 **
                                                        ** Oct 26 2010 Build **	
===========================================================================
1.  Added #defines to eipc_usersys.h to change the connection timeout 
    multiplier to either use a constant for all connection or a calculated
    values based on a minimum timeout in ticks. (was in 2.13, but got lost)
2.  Changed code to allow output only assemblies (no input data, just a HB)
3.  Simplified userobj example to just use assemblies
4.  Added file and moved PCCC to eips_userobj_pccc.c (NEW FILE)
5.  Added "EIPS_CIP_READ_WRITE_DATA_LOGIX" code to emulate a Logix (both
    from an actual Logix and our Tag Client)

===========================================================================
09/07/2010  REVISION 02.18 (Author: Jamin D. Wendorf)
===========================================================================
1.  Add C++ hook to allow C files in C++ projects.
2.  Seperated Client I/O defines from Client TCP defines.
3.  Removed C++ comments (//) replaced with C comments (a few new instances)

===========================================================================
08/20/2010  REVISION 02.17 (Author: Jamin D. Wendorf)
===========================================================================
1. Update client to use Class 3 explicit connections
2. Rewrote client to use more of a modular structure
3. Split files to only have one set of code with optional features (save $ and resouces)
4. De-coupled I/O from TCP in client
5. Fixed misc. bugs
6. Added better debug (file name and line)
7. Changed timer code to support names, better call backs and auto-figure the number of timers
8. Re-wrote example to be event driven, not based on RTA timers
9. Changed client process functions to include elapsed time (less timers)

===========================================================================
05/26/2010  REVISION 02.16 (Author: Jamin D. Wendorf)
===========================================================================
1. Added optional QoS Object (via #define EIPS_QOS_USED)
2. Added optional T2O 32-bit Run Time Header (via #define EIPS_USEROBJ_T2O_RUNTIME_HEADER_USED)
3. Changed default Input Only / Listen Only Heartbeats to 254/255

===========================================================================
02/05/2010  REVISION 02.15 (Author: Jamin D. Wendorf)
===========================================================================
1. Added "EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE" to allow variable assemblies.
   Once a connection is open, additional connections must use that size.
2. Modified eips_userobj_getAsmPtr() to only modify the passed size if
   it is too large or 0. This allows our validations to work.

===========================================================================
12/16/2009  REVISION 02.14 (Author: Jamin D. Wendorf)
===========================================================================
1. Re-certified for ODVA Conformance A-7 (server side only).
2. Removed unsupported additional error codes
3. Modified Multicast Algorith to match specification
4. Close TCP connection if Class 3 Explicit connection times out
5. Changed RTA contact information in copyright header

===========================================================================
05/04/2009  REVISION 02.13 (Author: Emily Brockman)
===========================================================================
1. Added basic support for the CIP Connection Configuration Object (0xF3)
2. #define will allow CCO Object to be included or excluded easily from build

===========================================================================
02/03/2009  REVISION 02.12 (Author: Jamin D. Wendorf) Re-certified with EDITT
01/22/2009  REVISION 02.12 (Author: Jamin D. Wendorf)
===========================================================================
1. Added support for multiple copies on Windows.
2. Removed C++ comments (//) replaced with C comments 
3. Conformed to A-6, Sept 17, 2008

===========================================================================
07/16/2008  REVISION 02.11 (Author: Jamin D. Wendorf)  ** Conform to A-6 **
                                                    ** Jun 24 2008 Build **		
===========================================================================
1. Added #define "EIPS_USER_MIN_TICK_RES_IN_USEC" to allow user to specify
   the minimum RPI, independent of the tick resolution supported.
2. Never allow 0 for the RPI.
3. Properly free connection resources on error.

===========================================================================
01/15/2008  REVISION 02.10 (Author: Jamin D. Wendorf)  ** Conform to A-5 **
===========================================================================
1.  Fixed explicit connection timeouts on Duplicate ForwardOpen and RPI=0
2.  Changed timer code to reserve timer 0
3.  Added better error handling for ForwardOpen and ForwardClose for
    objects that don't support then.
4.  Changed all copyright strings to include inc. after RTA
5.  Expanded PCCC to support strings and bits
6.  Added code to emulate 1761-NET-ENI (Rockwell Automation) to allow OCX
    devices to access our device via PCCC.
7.  Increase revision to 2.10 to allow for various beta code shipped and
    avoid any confusion.

===========================================================================
04/02/2007  REVISION 02.05 (Author: Jamin D. Wendorf)  ** Conform to A-4 **
===========================================================================
1.  Encap (Reg Session) shouldn't send response on (opt != 0)
2.  Changed CPF to handle all service code errors
3.  Support 16-bit Class and Instance ID's
4.  Changed "data" keyword to "data_ptr"

===========================================================================
06/06/2006  REVISION 02.04 (Author: Jamin D. Wendorf)
===========================================================================
1.  Fixed close error in client code (eipc_client.c).

===========================================================================
05/24/2006  REVISION 02.03 (Author: Jamin D. Wendorf)
===========================================================================
1.  Passed the EIP Workshop Recommendations at Plugfest 5.

===========================================================================
04/14/2006  REVISION 02.02 (Author: Jamin D. Wendorf)
===========================================================================
1.  Add PCCC OnDemand.
2.  Improve Client Example Code to separate all test cases.

===========================================================================
02/07/2006  REVISION 02.01 (Author: Jamin D. Wendorf)
===========================================================================
1.  Change all file headers to have a more complete copyright notice.
2.  Removed "EIPS_IO_SUPPORTED" since it is now a standard feature.
3.  Added code for EIP Workshop Recommendations.
4.  Added support for Client (#define) to simplify upgrades.
5.  Changed PCCC to pass all read and write requests to the user.
6.  Added Float support for PCCC.
7.  Added support for 8-bit, 16-bit and 32-bit Assemblies (#define)
8.  Added pass through support for all TCP and Ethernet Link Object 
    attributes. Also added Set capability where supported by ODVA.
9.  Added support for Input Only and Listen Only I/O connections.
10. Added support for multiple Configuration Assemblies.

===========================================================================
03/29/2004  REVISION 01.20 (Author: Jamin D. Wendorf)
===========================================================================
1.  Fixed minor conformance issues (A3.6).
2.  Fixed UDP interface to work with UDP implementations that don't use
    a socket id, just an IP Address and Port.

===========================================================================
03/26/2003  REVISION 01.01 (Author: Jamin D. Wendorf)
===========================================================================
1.  Sample Version - Cleaned up test code

*/

#endif /* __EIPS_REVISION_H__ */

