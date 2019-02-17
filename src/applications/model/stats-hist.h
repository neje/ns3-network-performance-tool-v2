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
 * Original version author: Pedro Fortuna  <pedro.fortuna@inescporto.pt> <pedro.fortuna@gmail.com>
 * Also based on basic-data-calculators.h by Joe Kopena (tjkopena@cs.drexel.edu)
 * Modified by: Nenad Jevtic (n.jevtic@sf.bg.ac.rs), Marija Malnar (m.malnar@sf.bg.ac.rs), 2018, University of Belgrade
 */

#ifndef NS3_STATS_HIST_H
#define NS3_STATS_HIST_H

#include <vector>
#include <stdint.h>
#include <ostream>
#include <cmath>
#include <string>

namespace ns3 {

/**
 * \brief Class used to store data and make an histogram of the data frequency.
 *
 * Data are grouped in "bins", i.e., intervals. Each value is assigned to the
 * bin according to the following formula: floor(value/binWidth).
 * Hence, bin \a i groups the data from [i*binWidth, (i+1)binWidth).
 *
 * This class only handles \a positive bins, i.e., it does \a not handles negative data.
 */

class StatsHist
{
public:

  // --- basic methods ---
  /**
   * \brief Constructor
   * \param binWidth width of the histogram "bin".
   */
  StatsHist (double binWidth = 1.0, std::string unit = "seconds");

  // Clear the Histogram Results
  /**
   * \brief Clear the Histogram Results.
   */
  void Clear ();

  // Methods for Getting the Histogram Results
  /**
   * \brief Returns the number of bins in the histogram.
   * \return the number of bins in the histogram
   */
  uint32_t GetNBins () const;
  /**
   * \brief Returns the bin start, i.e., index*binWidth
   * \param index the bin index
   * \return the bin start
   */
  double GetBinStart (uint32_t index) const;
  /**
   * \brief Returns the bin end, i.e., (index+1)*binWidth
   * \param index the bin index
   * \return the bin start
   */
  double GetBinEnd (uint32_t index) const;
  double GetBinMiddle (uint32_t index) const;
  /**
  * \brief Returns the bin width.
  *
  * Note that all the bins have the same width.
  *
  * \param index the bin index
  * \return the bin width
  */
  double GetBinWidth () const;
  /**
   * \brief Set the bin width.
   *
   * Note that you can change the bin width only if the histogram is empty.
   *
   * \param binWidth the bin width
   */
  void SetDefaultBinWidth (double binWidth);
  /**
   * \brief Get the number of data added to the bin.
   * \param index the bin index
   * \return the number of data added to the bin
   */
  uint32_t GetBinCount (uint32_t index) const;

  // Method for adding values
  /**
   * \brief Add a value to the histogram
   * \param value the value to add
   */
  void AddValue (double value);

  /**
   * \brief Provides raw estimate of the median based on histogram
   * \return estimated value of median
   */
  double GetMedianEstimation () const;

  /**
   * \brief Returns the data count
   * \return Count
   */
  long GetCount () const { return m_count; }
  /**
   * \brief Returns the sum
   * \return Total
   */
  double GetSum () const { return m_total; }
  /**
   * \brief Returns the minimum value
   * \return Min
   */
  double GetMin () const { return m_min; }
  /**
   * \brief Returns the maximum value
   * \return Max
   */
  double GetMax () const { return m_max; }
  /**
   * \brief Returns the mean value
   * \return Mean
   */
  double GetMean () const { return m_meanCurr; }
  /**
   * \brief Returns the standard deviation
   * \return Standard deviation
   */
  double GetStdDev () const { return std::sqrt (m_varianceCurr); }
  /**
   * \brief Returns the current variance
   * \return Variance
   */
  double GetVariance () const { return m_varianceCurr; }
  /**
   * \brief Returns the sum of squares
   * \return Sum of squares
   */
  double GetSqrSum () const { return m_squareTotal; }
  /**
   * \brief Print data to CSV file: 1. column is middle value of bin, 2. column is bin count
   */
  void WriteToCsvFile (std::string fileName, double writeBinWidth, std::string description = "Hist Data:") const;

private:
  std::vector<uint32_t> m_histogram; //!< Histogram data
  double m_binWidth; //!< Bin width
  std::string m_unit;

  long m_count;      //!< Count value

  double m_total;             //!< Total value
  double m_squareTotal;       //!< Sum of squares value
  double m_min;               //!< Minimum value
  double m_max;               //!< Maximum value r

  double m_meanCurr;     //!< Current mean
  double m_sCurr;        //!< Current s
  double m_varianceCurr; //!< Current variance

  double m_meanPrev;     //!< Previous mean
  double m_sPrev;        //!< Previous s
};

} // namespace ns3

#endif /* NS3_HIST_H */
