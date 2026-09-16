# Modelshop

A command line based tool for respawn models. (gui planned later)

Initially intended for just extracting 'dmx' (Data Model eXchange) files from reSource models, specifically Titanfall 2, it has massively grown in scope and features.

Currently the tool supports:
- extracting mesh data
- extracting animation data
- extracting vphysics data
- extracting map collision
- extracting ui panel topologies
- building an extensive qc

In the following model versions:
- 52 (Titanfall)
- 53 (Titanfall 2)
- 54 (Apex Legends) (NOTE: only mesh data for release build currently!)

For these formats:
- QC
- DMX
- SMD
- RMAX

Additional model versions are planned in the future, as well as potentially more export formats. Titanfall and Titanfall 2 are feature complete.

# Usage

The following commands are valid:

`-extract`: provides a path to a model, or directory with models in them.  
`-outpath`:	provides a path to a unique output directory.  
`-format`:	sets the desired export format with rmax 0, dmx 1, smd 2.  
`-version`:	sets the version to be used on files it cannot be determined from (Apex Legends).  
`-truncate_materials`: truncate material paths in exported files and qc if used.  
`-upaxis`: sets the dmx up axis.  
`-ignoremotion`: does not apply motion track to root bone of animations.  
`-ignoremesh`: skips exporting mesh data (models, phys, etc).  
`-ignoreanim`: skips exporting animation data.  
`-mergeuiverts`: sets the number of passes that should be done when merging ui panel vertices, default is 0 and does a single pass.  

`-qc_version`: sets the target version for qc files, should be in format "%hu %hu", "%hu" works but might be undefined behavior.  
`-qc_write`: qc file(s) will be exported.  
`-qc_use_includes`: makes qc export with include (qci) files.  
`-qc_use_trim_skins`: texture group will be trimmed to only changed materials.  

`-smd_version`: takes a number value of 1 to 3, sets the feature set for smd.   
`-dmx_version`: sets the feature set and formating on dmx, this should follow exactly as what is in a dmx file header.  

`-help`: prints the above in console.  