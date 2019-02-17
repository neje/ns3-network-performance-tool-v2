/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INESC Porto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author: Pedro Fortuna  <pedro.fortuna@inescporto.pt> <pedro.fortuna@gmail.com>
 * Modified by: Nenad Jevtic (n.jevtic@sf.bg.ac.rs), Marija Malnar (m.malnar@sf.bg.ac.rs), 2018, University of Belgrade
 */

#include <cmath>
#include <algorithm>
#include <fstream>

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/stats-hist.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StatsHist");

StatsHist::StatsHist (double binWidth, std::string unit) 
  : m_binWidth (binWidth),
    m_unit (unit),
    m_count (0),
    m_total (0.0),
    m_squareTotal (0.0),
    m_min (0.0),
    m_max (0.0),
    m_meanCurr (0.0),
    m_sCurr (0.0),
    m_varianceCurr (0.0),
    m_meanPrev (0.0),
    m_sPrev (0.0)
{
  if (m_histogram.size () != 0)
    {
      m_histogram.clear();
    }
}

void
StatsHist::Clear ()
{
  m_histogram.clear();
  m_count = 0;
  m_total = 0;
  m_squareTotal = 0;
  m_min = 0;
  m_max = 0;
  m_meanCurr = 0;
  m_sCurr = 0;
  m_varianceCurr = 0;
  m_meanPrev = 0;
  m_sPrev = 0;
}

uint32_t 
StatsHist::GetNBins () const
{
  return m_histogram.size ();
}

double 
StatsHist::GetBinStart (uint32_t index) const
{
  return index*m_binWidth;
}

double 
StatsHist::GetBinEnd (uint32_t index) const
{
  return (index + 1) * m_binWidth;
}

double 
StatsHist::GetBinMiddle (uint32_t index) const
{
  return (index+0.5)*m_binWidth;
}


double
StatsHist::GetBinWidth () const
{
  return m_binWidth;
}

void 
StatsHist::SetDefaultBinWidth (double binWidth)
{
  NS_ASSERT (m_histogram.size () == 0); //we can only change the bin width if no values were added
  m_binWidth = binWidth;
}

uint32_t 
StatsHist::GetBinCount (uint32_t index) const
{
  NS_ASSERT (index < m_histogram.size ());
  return m_histogram[index];
}

void 
StatsHist::AddValue (double value)
{

  uint32_t index = (uint32_t)std::floor (value/m_binWidth);

  //check if we need to resize the vector
  NS_LOG_DEBUG ("AddValue: index=" << index << ", m_histogram.size()=" << m_histogram.size ());

  if (index >= m_histogram.size ())
    {
      m_histogram.resize (index + 1, 0);
    }
  m_histogram[index]++;

  // borrowed from Joe Kopena
  m_count++;
  m_total += value;
  m_squareTotal += value*value;

  if (m_count == 1)
    {
      m_min = value;
      m_max = value;
    }
  else
    {
      m_min = (value < m_min) ? value : m_min;
      m_max = (value > m_max) ? value : m_max;
    }

  // Calculate the variance based on equations (15) and (16) on
  // page 216 of "The Art of Computer Programming, Volume 2",
  // Second Edition. Donald E. Knuth.  Addison-Wesley
  // Publishing Company, 1973.
  //
  // The relationships between the variance, standard deviation,
  // and s are as follows
  //
  //                      s
  //     variance  =  -----------
  //                   count - 1
  //
  //                                -------------
  //                               /
  //     standard_deviation  =    /  variance
  //                            \/
  //
  if (m_count == 1)
    {
      // Set the very first values.
      m_meanCurr     = value;
      m_sCurr        = 0;
      m_varianceCurr = m_sCurr;
    }
  else
    {
      // Save the previous values.
      m_meanPrev     = m_meanCurr;
      m_sPrev        = m_sCurr;

      // Update the current values.
      m_meanCurr     = m_meanPrev + (value - m_meanPrev) / m_count;
      m_sCurr        = m_sPrev    + (value - m_meanPrev) * (value - m_meanCurr);
      m_varianceCurr = m_sCurr / (m_count - 1);
    }
}

double 
StatsHist::GetMedianEstimation () const
{
  int64_t leftIndex=0, rightIndex=m_histogram.size ()-1;
  if (rightIndex==-1) return 0;
  if (rightIndex==0) return 0.5*m_binWidth;
  uint32_t leftCount=m_histogram[leftIndex], rightCount=m_histogram[rightIndex];

  while (rightIndex-leftIndex>1) //there are at least 1 bin between left and right index
  {
    if (leftCount<rightCount) // number of values in right bins is higher
      {
        leftIndex++; // reduce hist from the left
        leftCount+=m_histogram[leftIndex];
      }
    else if (leftCount>rightCount) // number of values in left bins is higher
      {
        rightIndex--; // reduce hist from the right
        rightCount+=m_histogram[rightIndex];
      }
    else // number of values in rigth and left bins is the same so far
      {
        if (rightIndex-leftIndex>2) // reduce hist from left and right if possibile 
          {
            leftIndex++;
            leftCount+=m_histogram[leftIndex];            
            rightIndex--;
            rightCount+=m_histogram[rightIndex];
          }
        else // there is only 1 been between left and right so this bin is solution
          {
            leftIndex++;
            return (leftIndex+0.5)*m_binWidth;
          }
      }
  }
  
  // At this point there is no more bins between lef and right side of reduced hist
  if (leftCount>rightCount) // left side has more values
    {
      return (leftIndex+0.5)*m_binWidth;
    }
  else if (leftCount<rightCount) // right side has more values
    {
      return (rightIndex+0.5)*m_binWidth;
    }
  else // number of values is the same in left and righ side of reduced hist
    {
      return rightIndex*m_binWidth;
    }
}

void
StatsHist::WriteToCsvFile (std::string fileName, double writeBinWidth, std::string description) const
{
  std::ofstream out;
  out.open (fileName.c_str (), std::ofstream::out | std::ofstream::app);
  out << description << std::endl;
  out << "Resolution: " << writeBinWidth << " " << m_unit << std::endl;
  out << "Bin:, Count:" << std::endl;

  if (writeBinWidth<=m_binWidth)
    {
      writeBinWidth = m_binWidth;
    }
  int mergeNBins = (int)(writeBinWidth / m_binWidth);

  // Reformat hist for writing
  uint32_t n = GetNBins ();
  uint32_t i = 0;
  uint32_t mergeSum=0;
  uint32_t mergeStartIndex=0, mergeStopIndex=0;
  while (i<n)
  {
    mergeSum=0;
    mergeStartIndex = i;
    mergeStopIndex = i+mergeNBins-1;
    for (uint32_t k=mergeStartIndex; k<=mergeStopIndex; k++)
      {
        if (i>=n) break;
        mergeSum += GetBinCount (i);
        i++;
      }
    if (mergeSum > 0)
      {
        out << GetBinStart (mergeStartIndex) + 0.5*(mergeNBins*m_binWidth) << "," << mergeSum << std::endl;
      }
  }
  out.close ();
}


} // namespace ns3


