# SpaceWalker

Here, we present SpaceWalker, an interactive visual analytics tool for exploration of large patches of Spatial Transcriptomic data. SpaceWalker consists of two key innovations: a real-time spatial exploration of the HD gene expression space, and a gradient gene detector for on-the-fly retrieval of locally varying genes from the full gene set. We complement this with a suite of user-defined visualization options to inspect and query the data, and to project analysis results on the spatial data.

This software is described in the paper [**SpaceWalker: Interactive Gradient Exploration for Spatial Transcriptomics Data**](https://www.biorxiv.org/content/10.1101/2023.03.20.532934v1)

# Installation
SpaceWalker is built as a plug-in for the ManiVault application building system as described in [**ManiVault: A Flexible and Extensible Visual Analytics Framework for High-Dimensional Data**](https://arxiv.org/abs/2308.01751).

The installer of this software which includes the SpaceWalker plugin can be downloaded below:

[![Button Icon]][Link]

Currently, only a Windows installer is provided, Linux and Mac will follow in the coming period.

# Example Datasets
In the paper several datasets are discussed which are available here for exploring locally via downloading a ManiVault project file.

smFISH           |  HybISS
:-------------------------:|:-------------------------:
<img src="https://github.com/ManiVaultStudio/SpaceWalker/assets/2978176/08e17665-0b0c-48aa-8e76-9a39b8093f69" width="90%" /> [![Download Icon]][SMLink] |  [![Download Icon]][HybLink]
**EEL FISH** | **ABC Atlas**
<img src="https://github.com/ManiVaultStudio/SpaceWalker/assets/2978176/cc487851-6af5-41b5-98a8-c85a1d21d8e0" width="90%" /> [![Download Icon]][EELLink] | <img src="https://github.com/ManiVaultStudio/SpaceWalker/assets/2978176/e473439c-471d-4c9d-a4a6-b887f3e81838" width="90%" /> [![Download Icon]][ABCLink]

<!---------------------------------------------------------------------------->
[Link]: https://github.com/ManiVaultStudio/Installer/releases 'Download the Installer'
[SMLink]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/SpaceTx_smFish_MerFish.mv?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'smFISH'
[HybLink]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/HyBISS.mv?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'HybISS'
[EELLink]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/EELFISH.mv?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'EEL FISH'
[ABCLink]: https://www.biorxiv.org/content/10.1101/2023.03.20.532934v1 'EEL FISH'
<!---------------------------------------------------------------------------->
[Button Example]: https://img.shields.io/badge/Title-37a779?style=for-the-badge
[Button Icon]: https://img.shields.io/badge/Installation-EF2D5E?style=for-the-badge&logoColor=white&logo=DocuSign
[Download Icon]: https://img.shields.io/badge/Download-EF2D5E?style=for-the-badge&logoColor=white&logo=DocuSign
[#]: #

# License
SpaceWalker is licensed under LGPL v3.0, refer to the LICENSE file in the top level directory.

Copyright © 2023 BioVault (Biomedical Visual Analytics Unit LUMC)
