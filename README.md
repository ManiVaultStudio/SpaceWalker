# SpaceWalker
[![DOI](https://zenodo.org/badge/684539908.svg)](https://zenodo.org/doi/10.5281/zenodo.10017489)

Here, we present SpaceWalker, an interactive visual analytics tool for exploration of large patches of Spatial Transcriptomic data. SpaceWalker consists of two key innovations: a real-time spatial exploration of the HD gene expression space, and a gradient gene detector for on-the-fly retrieval of locally varying genes from the full gene set. We complement this with a suite of user-defined visualization options to inspect and query the data, and to project analysis results on the spatial data.

This software is described in the paper [**SpaceWalker: Interactive Gradient Exploration for Spatial Transcriptomics Data**](https://www.biorxiv.org/content/10.1101/2023.03.20.532934v1)

# Installation
SpaceWalker is built as a plug-in for the ManiVault application building system as described in [**ManiVault: A Flexible and Extensible Visual Analytics Framework for High-Dimensional Data**](https://arxiv.org/abs/2308.01751).

The installer of this software which includes the SpaceWalker plugin can be downloaded below:

[![Button Icon]][Link]

Currently, only a Windows installer is provided, Linux and Mac will follow in the coming period.

# Example Datasets
In the paper several datasets are discussed which are available here for exploring locally via downloading a ManiVault project file.

smFISH & MERFISH<sup>1</sup> |  HybISS<sup>2</sup>
:-------------------------:|:-------------------------:
![image](https://github.com/ManiVaultStudio/SpaceWalker/assets/2978176/7b17e46c-177a-4b5d-9764-16627b36f46f) [![Download Icon]][SMLink] | ![image](https://github.com/ManiVaultStudio/SpaceWalker/assets/2978176/cf43ae47-25ad-48f4-b9a0-9c19a69a8eba) [![Download Icon]][HybLink]
**EEL FISH<sup>3</sup>** | **ABC Atlas<sup>4</sup>**
![image](https://github.com/ManiVaultStudio/SpaceWalker/assets/2978176/341c7bd6-4f7a-405e-8763-f049fa5b02f2) [![Download Icon]][EELLink] | ![image](https://github.com/ManiVaultStudio/SpaceWalker/assets/2978176/867a6724-dd5e-4738-8bee-f3351ae94639) [![Download Icon]][ABCLink]

<!---------------------------------------------------------------------------->
[Link]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/ManiVault_spacewalker_offline.exe?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'Download the Installer'
[SMLink]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/SpaceTx_smFish_MerFish.mv?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'smFISH'
[HybLink]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/HyBISS.mv?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'HybISS'
[EELLink]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/EELFISH.mv?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'EEL FISH'
[ABCLink]: https://www.dropbox.com/scl/fo/18g5gmg7o54k48h6dvdhv/h/SpaceWalker_ABCATLAS_Saved.mv?rlkey=31e1jxw3tl2wqtdcw8u1ptd2v&dl=0 'ABC Atlas'
<!---------------------------------------------------------------------------->
[Button Example]: https://img.shields.io/badge/Title-37a779?style=for-the-badge
[Button Icon]: https://img.shields.io/badge/Installation-EF2D5E?style=for-the-badge&logoColor=white&logo=DocuSign
[Download Icon]: https://img.shields.io/badge/Download-EF2D5E?style=for-the-badge&logoColor=white&logo=DocuSign
[#]: #

## Data sources
1. Long, B., Miller, J., and The SpaceTx Consortium. (2023). SpaceTx: A Roadmap for Benchmarking Spatial Transcriptomics Exploration of the Brain. arXiv preprint arXiv:2301.08436. https://viewer.cytosplore.org
2. La Manno, G., Siletti, K., Furlan, A., Gyllborg, D., Vinsland, E., Mossi Albiach, A., Mattsson Langseth, C., Khven, I., Lederer, A.R., and Dratva, L.M. (2021). Molecular architecture of the developing mouse brain. Nature 596, 92-96. http://mousebrain.org/development/downloads.html
3. Borm, L.E., Mossi Albiach, A., Mannens, C.C., Janusauskas, J., Özgün, C., Fernández-García, D., Hodge, R., Castillo, F., Hedin, C.R., Villablanca, E.J., et al. (2022). Scalable in situ single-cell profiling by electrophoretic capture of mRNA using EEL FISH. Nature Biotechnology, 1-10. http://mousebrain.org/adult/downloads.html
4. Yao, Z., van Velthoven, C.T., Kunst, M., Zhang, M., McMillen, D., Lee, C., Jung, W., Goldy, J., Abdelhak, A., Baker, P., and Barkan, E. (2023). A high-resolution transcriptomic and spatial atlas of cell types in the whole mouse brain. bioRxiv, 2023.2003. 2006.531121. https://github.com/AllenInstitute/abc_atlas_access/blob/main/descriptions/MERFISH-C57BL6J-638850.md



# License
SpaceWalker is licensed under LGPL v3.0, refer to the LICENSE file in the top level directory.

Copyright © 2023 BioVault (Biomedical Visual Analytics Unit LUMC)
