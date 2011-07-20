#!/usr/bin/python
##===========================================================================##
#  Copyright (C) 2008 Los Alamos National Security, LLC. All Rights Reserved. #
#               Author: Samuel K. Gutierrez - samuel[at]lanl.gov              #
# Copyright (c) 2008-2009 Krell Institute  All Rights Reserved.               #
#               Author: Additional changes added by jeg                       #
##===========================================================================##

#=============================================================================#
# This program is free software; you can redistribute it and/or modify it     #
# under the terms of the GNU General Public License as published by the Free  #
# Software Foundation; either version 2 of the License, or (at your option)   #
# any later version.                                                          #
#                                                                             #
# This program is distributed in the hope that it will be useful, but WITHOUT #
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       #
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   #
# more details.                                                               #
#                                                                             #
# You should have received a copy of the GNU General Public License along     #
# with this program; if not, write to the Free Software Foundation, Inc., 59  #
# Temple Place, Suite 330, Boston, MA  02111-1307 USA                         #
#=============================================================================#

#LANL's Yellowrail MRNet site.py

import commands
import warnings 
from re import compile
warnings.simplefilter('ignore', DeprecationWarning)
from sets import Set
warnings.resetwarnings()
from sys import stdout
import os
import sys
from os import path

# Flag indicating if debugging output will be displayed.
debug = False

#Flag indicating if a topology file update is needed
topFileUpdateNeeded = True

# Regular expression string utilized to help parse qstat output.
# jeg commented this out and replaced it with a scheme that takes the 
# output of uname -n and strips all numeric characters off the right side.

hostnodeName = os.uname()[1].split('.')[0]

if debug:
   print "hostnodeName=%s" % hostnodeName

hostnodeNameStrip = hostnodeName.rstrip('0123456789')
if debug:
   print "stripped hostnodeNameStrip=%s" % hostnodeNameStrip

frontEndQstatRegEx = hostnodeNameStrip + '[^/*]*'

if debug:
   print "stripped frontEndQstatRegEx=%s" % frontEndQstatRegEx

# Location of user-specific CBTF preferences: $HOME/.cbtf
cbtfuserpref = os.environ['HOME'] + os.sep + '.cbtf'

def getAllocatedNodesString():
    if os.environ.has_key('PBS_JOBID'):
        allocnodes = commands.getoutput('cat $PBS_NODEFILE  | uniq')
        if debug:
            print ' '
            #print "from getRawAllocatedNodesString, uniqallocnodes=%s" % uniqallocnodes
    else:
        print 'fatal error...PBS_JOBID not defined.'
        sys.exit()
    if debug:
       print "From getRawAllocatedNodesString, allocnodes=%s" % allocnodes
    return allocnodes

def prepENV(topologyStringHash):
    global topFileUpdateNeeded
    
    if debug:
        sys.stdout.write('checking for ' + cbtfuserpref + ' ...')
    
    # Check for presence of User-Specific CBTF Preference Directory:
    # $HOME/.cbtf.  We are assuming that this is the most
    # convenient place for autogenerated MRNet topology files.
    if os.path.isdir(cbtfuserpref):
        if debug:
            print 'present'
            sys.stdout.write('topology update needed...')
        
        #Overwrite CBTF_MRNET_TOPOLOGY_FILE with:
        #$HOME/.cbtf/HOSTNAME.<topologyStringHash>.top
        #newtopenv = cbtfuserpref + os.sep + os.uname()[1].split('.')[0] + \
        #'.' + topologyStringHash + '.top'
        newtopenv = cbtfuserpref + os.sep + 'cbtf_topology'
        
        os.environ['CBTF_MRNET_TOPOLOGY_FILE'] = newtopenv
        print 'created topology file: ' + newtopenv
    else:
        if debug:
            print 'not present'
            sys.stdout.write('topology update needed...')
        
        #Make Directory $HOME/.cbtf
        os.mkdir(cbtfuserpref, 0755)
        
        #os.environ['CBTF_MRNET_TOPOLOGY_FILE'] = \
        #cbtfuserpref + os.sep + os.uname()[1].split('.')[0] + \
        #'.' + topologyStringHash + '.top'

        os.environ['CBTF_MRNET_TOPOLOGY_FILE'] = \
        cbtfuserpref + os.sep + 'cbtf_topology'

        print 'created topology file: ' + cbtfuserpref + os.sep + 'cbtf_topology'

    if os.path.isfile(os.environ['CBTF_MRNET_TOPOLOGY_FILE']):
        if debug:
            print 'no'
    else:
        if debug:
            print 'yes'
        topFileUpdateNeeded = True
    if debug:
        print ('CBTF_MRNET_TOPOLOGY_FILE: ' + \
        os.environ['CBTF_MRNET_TOPOLOGY_FILE'])

## haveCBTFPrefix()
# Returns True if $CBTF_PREFIX is a directory that exists.
# Returns False otherwise.
def haveCBTFPrefix():
    if os.getenv('CBTF_PREFIX'):
       return os.path.exists(os.environ['CBTF_PREFIX'])
    else:
       return False

## getCBTFPrefix()
# Returns the path pointed to by $CBTF_PREFIX.
# Returns NULL otherwise.
def getCBTFPrefix():
    if os.getenv('CBTF_PREFIX'):
       return os.path.realpath(os.environ['CBTF_PREFIX'])
    else:
       return NULL

## haveTopgen()
# Returns True if mrnet_topgen is present in $CBTF_PREFIX/bin.
# Returns False otherwise.
def haveTopgen():
    return os.path.isfile(os.environ['CBTF_PREFIX'] + os.sep + 'bin' + \
                        os.sep + 'mrnet_topgen')

#FIXME
## generateMRNetTopologyString(degree, numleaves)
# Returns mrnet_topgen-based MRNet topology string.
def generateMRNetTopologyString(degree, numleaves):
    mrntstr = 'echo "' + getAllocatedNodesString() + '" | ' + \
				'mrnet_topgen -b %dx%d' % (degree, numleaves)
    
    #Capture generated MRNet topology string
    mrntopstr = commands.getoutput(mrntstr)
    
    if debug:
        print mrntopstr

    return mrntopstr
    
def createTopologyFile(topologyString):
    #Make certain CBTF_MRNET_TOPOLOGY_FILE is present

    if debug:
       print "In createTopologyFile, topologyString=%s" % topologyString

    if os.environ.has_key('CBTF_MRNET_TOPOLOGY_FILE'):
        try:
            topfile = open(os.environ['CBTF_MRNET_TOPOLOGY_FILE'], 'w')
            topfile.write(topologyString)
            topfile.close()
        except:
            exc_info = sys.exc_info()
            print exc_info[1]
            print ('an error was encountered during MRNet topology file ' +
            'generation...')
            sys.exit()
    else:
        print ('CBTF_MRNET_TOPOLOGY_FILE environment variable ' +
        'not defined...')
        sys.exit()
    
def getAllocatedNodeCount():
    rlnodeinfo = commands.getoutput('qstat -f $PBS_JOBID | ' + 
    'grep Resource_List.nodes')
    return int(rlnodeinfo.split(' ')[-1].split(':')[0])
     
def getAllocatedNodePPNCount():
    rlnodeinfo = commands.getoutput('qstat -f $PBS_JOBID | ' +
    'grep Resource_List.nodes')
    
    return int(rlnodeinfo.split(' ')[-1].split('=')[-1])

##generateSimpleTopologyString() 
def generateSimpleTopologyString():
    #Strip .lanl.gov
    hostname = os.uname()[1]
    hostname = hostname.split('.')[0]
    
    return hostname + ':0 => \n' + '  ' + hostname + ':1 ;'

#TODO:FIXME 
def generateSimpleBETopologyString():
    nodelist = getAllocatedNodesString().split('\n')
    
    topstring = nodelist[0] + ':0 =>\n  ' + nodelist[0] + ':1'


    #count = 2
    for node in nodelist[1::1]:
        topstring += '\n  ' + node + ':0'
    #    topstring += '\n  %s:%d' % (node, count)
    #    count += 1
    
    topstring += ' ;'
    return topstring

#TODO:FIXME 
def generateSimpleSLURMBETopologyString():
    if debug:
       print "generateSimpleSLURMBETopologyString, hostnodeName=" + hostnodeName
    slurmnodescmdstr = "srun " + "/bin/hostname" + " | sort | uniq"
    if debug:
       print "generateSimpleSLURMBETopologyString, slurmnodescmdstr=" + slurmnodescmdstr
    nodelist = commands.getoutput(slurmnodescmdstr).split('\n')
    if debug:
       print "generateSimpleSLURMBETopologyString, begin nodelist=" 
       print nodelist
       print "generateSimpleSLURMBETopologyString, end nodelist" 
    
    if debug:
       print "generateSimpleSLURMBETopologyString, first reference, begin topstring=" 
    topstring = hostnodeName + ':0 =>\n  ' + hostnodeName + ':1'
    if debug:
       print topstring
       print "generateSimpleSLURMBETopologyString, first reference, end topstring" 

    for node in nodelist[0::1]:
        if (node != hostnodeName):
          if debug:
             print "generateSimpleSLURMBETopologyString, in for loop, begin topstring=" 
             print topstring
             print "generateSimpleSLURMBETopologyString, in for loop, end topstring" 
          topstring += '\n  ' + node + ':0'
    
    topstring += ' ;'

    if debug:
       print "generateSimpleSLURMBETopologyString, RETURN begin topstring=" 
       print topstring
       print "generateSimpleSLURMBETopologyString, RETURN end topstring" 

    return topstring


# Overkill
def getStringHashValue(inputString):
    return hex(hash(inputString))
# main()
def main():

    #Make sure that .cbtf is created before used
    if os.path.isdir(cbtfuserpref):
        if debug:
            print '.cbtf directory exists'
    else:
        #Make Directory $HOME/.cbtf
        os.mkdir(cbtfuserpref, 0755)
        if debug:
            print '.cbtf directory created'

    #Assuming presence of PBS_JOBID is a good 
    #indicator that we are on compute nodes
    if os.environ.has_key('PBS_JOBID'):
        if debug:
            print 'on compute node...'
        prepENV(getStringHashValue(generateSimpleBETopologyString()))

        if debug:
            print ""
            print "An MRNet Configuration has been created for you - please"
            print "check if this is consistent with your partition."
            print ""
            print "Current MRNet configuration in " + cbtfuserpref
            print ""
            print generateSimpleBETopologyString()
        if debug:
           print "after print of generateSimpleBETopologyString()"

        #TODO:FIXME Simple hack to get things going
        if(topFileUpdateNeeded):
            createTopologyFile(generateSimpleBETopologyString())

    #Assuming presence of SLURM_JOBID is a good 
    #indicator that we are on compute nodes
    elif (os.environ.has_key("SLURM_JOBID")):
        if debug:
            print 'USING SLURM ... from node ' + hostnodeName

        prepENV(getStringHashValue(generateSimpleSLURMBETopologyString()))
        #TODO:FIXME Simple hack to get things going
        if(topFileUpdateNeeded):
            createTopologyFile(generateSimpleSLURMBETopologyString())

    #If PBS_JOBID or SLURM_JOBID is not present, then we
    #better be on a compile node...
    else:
        if debug:
          if(haveCBTFPrefix()):
            print 'We have CBTF_PREFIX set'
          else:
            print 'We do NOT have CBTF_PREFIX set'

          if(haveCBTFPrefix()):
              print 'CBTF_PREFIX=' + getCBTFPrefix()

        if debug:
            print 'on compile node...' \
                  + hostnodeName

        # create list of nodes.
        cmdstr = "uname -n > " + cbtfuserpref + os.sep + ".cbtf-mrnet-hosts"
        os.system(cmdstr);
 
        #Make sure we have mrnet_topgen before we continue. 
        #It's not used here, but will be.??
        prepENV(getStringHashValue(generateSimpleTopologyString()))
        #print generateSimpleTopologyString()
                #TODO: FIXME Simple hack to get things going
        if(topFileUpdateNeeded):
            createTopologyFile(generateSimpleTopologyString())

if __name__ == '__main__' :
    #Used to suppress python hex() Future Warning message.
    warnings.filterwarnings('ignore')
    main()
