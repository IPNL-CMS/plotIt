#!/bin/bash

mkdir plots_for_note

for btag in 1btag 2btag ; do
  for name in ElectronPt ; do
    cp plots/${btag}/Electron_pt.pdf plots_for_note/dataMC_${name}_semie_${btag}.pdf
  done
  
  for name in MuonPt ; do
    cp plots/${btag}/Muon_pt.pdf plots_for_note/dataMC_${name}_semimu_${btag}.pdf
  done
  
  for channel in semie semimu ; do
    for name in MET firstjet secondjet thirdjet fourthjet nJets nBJets; do
      cp plots/${btag}/${name}_${channel}.pdf plots_for_note/dataMC_${name}_${channel}_${btag}.pdf
    done
    for name in Mtt ; do
      cp plots/${btag}/mtt_${btag}_${channel}.pdf plots_for_note/dataMC_${name}_${channel}_${btag}.pdf
      cp plots/${btag}/mtt_${btag}_${channel}_log.pdf plots_for_note/dataMC_${name}_${channel}_${btag}_log.pdf
    done
  done
done

