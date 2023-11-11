#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#ifdef __BMI2__
#include <x86intrin.h>
#endif

#define STRINGIFY0(x) #x
#define STRINGIFY(x) STRINGIFY0(x)

alignas(64) constexpr std::uint64_t piece_hash_values[768] =
{
    0x01510564b3120641, 0x81d4176bde347ed0, 0x87bb68b4058d6c0e, 0x2657ac0ae9cfb089, 0x239bb0832ef29261, 0x59d1f0891d937542,
    0x7bf5a56b5193f6d1, 0xb01171984a82d54d, 0x684643adfab5dd36, 0x56581113fe97511d, 0x2750e3fa9ec1d77e, 0xd8437d4ac7495dff,
    0x58a65f4d978060a0, 0x3a3b82f9e5ac5de9, 0x84e70af5088a1209, 0x8b3a40f05d78df64, 0x50abb939d84a6026, 0xb78a7755ebe3e565,
    0xc066557972943f9a, 0x259df84b0a47c5e8, 0x828093ffd06a1e1d, 0xf14197c74415b39c, 0xc832ec8e3807bc28, 0x6829333466959dc0,
    0x1fae99f759989009, 0x228e7a20d3426dd2, 0xa68fceecf12c9f6e, 0xc3ac6465509ab297, 0x7c6715faa2c82db6, 0x9a922a5bed65a45f,
    0x21bc4985d9891f14, 0xdbba65e4cce516db, 0x168bc1ac8656f100, 0xb87964bab9fb01c8, 0xd4aa666d5d6aed6f, 0x6282a6d326cf4dd3,
    0x93bbaf24e730ca68, 0xae108384900df125, 0xe88157bb4310d894, 0xfa73dac1e0955aac, 0x44d834b4429548d5, 0xd97102339b797bc7,
    0xbd4a027657a3d40b, 0xd7ad8659afd827b2, 0x0bfe7f624ce63053, 0xfaec5ca7cb9ae651, 0x0827785fff637176, 0xfbed2a0e957bb521,
    0xedc57147531e0638, 0xf5438f94341590e7, 0xb65c5c791a4b22a5, 0x6af1d8c342571620, 0xc22f6304656edfa1, 0x668250c6d0c9ba29,
    0xf9563f62fe1a2986, 0xb388c9b3a5c8c817, 0xe4fb0da0fcfe9120, 0x2f70d87a5dc77f9e, 0x6cec6c1e73663162, 0x1774e45e8b41fe77,
    0x4a7512fae5056cd7, 0x778a407b931e0d10, 0x29f213a85324ff78, 0x39b1fd85666266d2, 0x28e5418a1d647893, 0xedbe1e7ed4e59d39,
    0x912d60d1c7310467, 0x320276f756354804, 0xf0d15dee2937cfb7, 0xfe66c5c1ece7da56, 0x7ff5d127c7637c98, 0x635a35864b616e95,
    0x3b7ff5332c916052, 0x60b441c681039925, 0x623f0ef9745357f7, 0x8e385524f0fee030, 0xc6558ac6f0e2f2b8, 0xff9ac67ea6add597,
    0x52d1d0845eb36202, 0x47458acad9057be0, 0x84216f9b80bd1f22, 0x74250c3e45b0c68c, 0x4d6b6df316d33a43, 0xc94ceb3403517aff,
    0xb2d5b8a79f7e8f0b, 0x1ecdd45a15070fbe, 0xb3dadee753266cc0, 0xbe53d1a47fcc874b, 0x73ed0f65273f6701, 0xe01b7e80cc003a38,
    0x65e952b821f47507, 0xd286cea65a93775b, 0x8a5fb38ca8847d26, 0xbd4e3862ea3944f3, 0x6a8dcd269863193f, 0x4337598bf7880af1,
    0x392861bedebfa307, 0x16e6415a14a3d428, 0x3375b23fad63615b, 0x5117b4c9a562d7f5, 0x6fee5dd92e02d606, 0xaccbdf3d9faeb9d5,
    0x6fcef16367915bc9, 0xf6bad1d14b341fda, 0xa1b783d18ec50f37, 0x499bef485300e8f5, 0x98fe80bf0496e3eb, 0xadfc4f96e26a5ab7,
    0xe2b0ed4905977281, 0x973c48c75fb999f7, 0xc23ee3f1354ccef9, 0x8f6fc65979ad2834, 0xd45600d23c57f0ce, 0x2b1084d79964c69f,
    0xa491e16684c8ec15, 0xcb0dd5e635ad3d67, 0xbeec5f1b166309ce, 0x3360f9966d55a43d, 0x078af4cf6104fce8, 0xf57a0fa91d9b35b1,
    0xb453cc82ae056760, 0x6647a9b607758562, 0x469bcfcace5e1fe2, 0x04152c1ee1d98ada, 0xd8bf1575c3478f56, 0x621243b3ed338663,
    0x1a4aac88992e89e7, 0x94b5b7c8dbe96476, 0x44d2c61679918c41, 0x453333a65affeb45, 0x877ad20baf7b780e, 0xf702da39c31fcd7e,
    0x01a58bf8b4cf1741, 0x51ad6cdc353b2e24, 0x435ab9182ca60b86, 0xaf5e9ae740c75cb2, 0x8c0b09c002858165, 0x65c9319e2b9de475,
    0xbdbd6796f98a3f18, 0x1bb6c455bd0fecd1, 0xec942cd0436357df, 0x4341b648b8b09eb5, 0xb71b2e0d9cb553cc, 0x9b7517502d370063,
    0xb187cdca15cdb533, 0x5cdf00802b4143b3, 0x6d90c827ff455de8, 0x9482d9c7b7157d3c, 0x7dcf88f29165a2b8, 0x7f38c2720ca4e751,
    0x5e717fdd22de6d9c, 0xa07db609449c7bba, 0xe668ada5b88b1e4d, 0xc6dbe293c52f0062, 0x8854a1bc2f80e158, 0x7fdb199438e77b63,
    0x99df87c5619e08b1, 0xe207a34f29afd1da, 0x9dd8218c8b7058d4, 0x14d4dac5564a6259, 0x91951b8f2fa61188, 0xf7b66f8da3b61ad1,
    0x15605b78b428db4c, 0x3f42707c036f7cd1, 0x7f5926e928569838, 0x1c9e99827b2d6099, 0xe33a8a008676c316, 0x3bb08b9f5cb23072,
    0xd05c1cfcabed8dc5, 0x3a14e044b71e52b9, 0x0cc6c4a45989311c, 0xafb03a0b442d74e3, 0x3524c18828f454d9, 0x171c6965cfdb3181,
    0x5c4b6cd2081248af, 0x4accbb4917b8f623, 0x8a02784fbdde85c8, 0xc32a3a4ee86baadb, 0xa230812412e2382c, 0xddfa5b53b7509279,
    0x6c3160a9772508f3, 0xf671893554f7b500, 0xe77fcfbe6af88f1b, 0x258658ba358c623c, 0x6516fe0dd347a3aa, 0xaee37e3a9a5e0b73,
    0x8cfa783ae3077176, 0x9cea69b36ad98573, 0xd513a19def5b5730, 0xf8c2027a2454d565, 0xdef5bff34a39c6ae, 0xb9b078405b1a0120,
    0xa52193bbc786eabb, 0xdbb9e96ce7794f1f, 0x8eceeb4160990c05, 0xc440b57ef9fa3c69, 0xb4329e512724b2fd, 0x0b703bbd9bedbd9f,
    0x285464cf7ed168c6, 0xcaabe79357398458, 0x0a48ce323e28c8f2, 0xf43e8d8939267703, 0x3a8ab7231a83156b, 0x20a72698a4016d0f,
    0xebf94c66fa4b8a7c, 0xeb43bd945cd9fdbf, 0xa21a4e3d047a6b8c, 0xacb26449c703b63e, 0x1eff980ddd7b0b6b, 0x4e73f4b3adfa97db,
    0xe1e95701094a99d7, 0xf554dc731963faaf, 0xc53c4761c6868c1b, 0x70c86d4c1f64e7f7, 0x7720a6919b003639, 0xe5899a07684c118c,
    0xff516cc925e7ad1c, 0xe21ae7692507eaaa, 0xfea5136a2544e49d, 0xd7ca22ad86837cc4, 0xab9f3e99a2183901, 0xfcbfe9696892a996,
    0x670ae19d64700bf8, 0xd7b0e0062a1c8d89, 0xc733362135bb62df, 0x7ffcadd0ccfba35d, 0x1687731f1682a050, 0x7d73d385542c51e8,
    0x6655450cb789a73a, 0x96d7b1dbe3be0834, 0xf69415a8209fea0a, 0x447df78306022926, 0x59af5641602c6f1f, 0x2909c84c90d1fbc9,
    0x641a859a67915b47, 0xc4d66647838111bd, 0x23c0550f496b18dc, 0x74d0dfbfb36e6c4e, 0x2a8171f446261884, 0xdf67aa84b9386950,
    0x876183a15882056a, 0x41e5b1f1d3ac62cd, 0xd75dea8cd3da9324, 0x29a5623c5a73b2c1, 0xf03495a745f57adc, 0x3677fee28f712777,
    0x7e079e625ebcd643, 0xfd9c7e3873fa4968, 0xc0b88969b10fea00, 0x162b32c51b4f274a, 0x8fa2fe4d6fe8749b, 0xfbaa61cb520b7d64,
    0xc9cd8ebe6dd058e6, 0x1e11558fdad4f19b, 0xa74add0d19e28b5e, 0xb0ff88a37b32e2aa, 0x9c4d477dc95d6c35, 0x553341f667a39262,
    0x410e2cd1551165af, 0x6348886902f0facd, 0xc09ce6b1ace5ac4a, 0x3174637eb153f231, 0x365dd77836997349, 0x4dbbe96960932d33,
    0x6f12002a6a3d6569, 0x4b180d5d0ac02e13, 0x82e5b0283d65d689, 0x840a38dcb08ed8f5, 0x404e1835a803077a, 0xf9ae829427d1c668,
    0x33b2b5e17dbb13c5, 0xf2de0ea2baccff67, 0x5ddfcfd969e05967, 0x2cebdef3012af72c, 0xc69fc0b7145fb7b1, 0xc385cb710987908f,
    0x78bcfd7a7998d4bd, 0x45436835c9e71530, 0xce5d083de2e45ad9, 0x16dadc0e83ea3c03, 0xb88c24af99ba6e4e, 0x0860e4f968817979,
    0x0b56f9f3735cb1e7, 0x6cfddd80edadfcb6, 0x52cd8835149de29a, 0x6cc7e1c786cfdc87, 0x584039c93af14c30, 0x27b216c4bd930044,
    0xb3d6b96f467d9ecc, 0x31a553f80b2e5022, 0x415f6790ac607d45, 0xb89849daf7b47e89, 0xf328d117d2abd052, 0x122b4562f53677e3,
    0x7347a87413a3d6d3, 0xf83d0675499b95c5, 0xb661cf20734bf9c3, 0xa2c90338026abe16, 0xfe63b8f7c28e55fd, 0xd7d61c7853480971,
    0x62ab139d86a25e05, 0x038104a92af36a08, 0x26dbea15f190ff8e, 0x76c503cdf19c1b34, 0x39493d7f95510fa6, 0x3de7677063d8ca4e,
    0x563224c4344b1f49, 0x88cac88522a207b1, 0x61186d8f83e2de0a, 0x57c78abab0b10468, 0x76fc69eb765a0ebd, 0xdb3bfe6ebca12b66,
    0x32b7c9b9aa5db3e0, 0x8cf3098608ae54c7, 0x6377bf779a22d35e, 0xb8ced088da41a64b, 0x7b9dfc4f457645a4, 0xe10f95ed42a71ca3,
    0x83d7c3bda0091ee9, 0x8a5eedead8bcee3f, 0x97bb2b8cd06054d5, 0xcace0493f56dde7a, 0x5e19557291ce761d, 0x35e2e331a00c7cc6,
    0x8e479e980b774ccd, 0xd81f9d6da98a7b75, 0x040fe9903e5ef705, 0xc84d7e7977177e99, 0xc1b3ca8fde57beff, 0x4d4c1bc43487042c,
    0xc1cfd0b9987d9c4e, 0xee952559b98f76dc, 0x58e9ab1fac68df56, 0x4778b24c35aa319a, 0xfab4b93c375e11ea, 0x4246b2f9dfd20f5f,
    0x0548db9b99a09f21, 0xb20deafb83f15dd4, 0xf1406b9b45ded409, 0x1e97c4b64a0b34dd, 0x0017f1c5c3522b0a, 0xabf8418bfa6b5e1e,
    0x85e4ed8013199ae5, 0x8a68fbff4cecbd2a, 0xc78dac74332fb46e, 0xcff4667a1a1b32b3, 0x4388ccd47fdab2a1, 0x1a2878fa90bca07d,
    0xb34de9654406e798, 0xe0eb5106073f363b, 0x36b914eede5fb749, 0xae1d8832c8feffe1, 0x6141f1727a406dd5, 0xe199bc4547707d95,
    0x921c3ecc23454d02, 0x4196b9da5ba5649b, 0xa292848e298ca43a, 0x97c102c99952c0e8, 0xa38eb4233b610314, 0xbe699659ed7646f5,
    0xad6f2838898aa8b2, 0x0a8aa05f619859a2, 0xd076986eedc21a10, 0x908f308a67c8e76d, 0xbf49e36c568564fd, 0xf1a377959a43dcbc,
    0x65462d59f03f0131, 0x30dc000c3fe2ead5, 0x730c7689a974b7ec, 0xd73663fa0953acec, 0x6bb33feb6de8e97e, 0x587d46787801c736,
    0xa0d15a6847936f7e, 0x54db8b2af187c3b5, 0x9894a6dd3c77975a, 0xad73d67c6908a76d, 0xc3695f0912b83595, 0x1c68ed3fa419ac88,
    0x347c6a8ec355f4d7, 0xf5c028c289dd3205, 0x12046064aa399811, 0xd36d43590abc4e6e, 0x54dcf2d55de9f391, 0x96b853638c5d0871,
    0xc35409044f5c84cf, 0x7f83a2320acc30c9, 0x64d9adc60f3bfc05, 0xc408fad04d9a11e3, 0x732630d9bb103d05, 0x531a6e3d82f3d416,
    0xe859e6cd20108912, 0x1ab7394bab5913b7, 0x579f8ef2e96611a2, 0x18146b5bdbafab0e, 0x42ab1933dbd07b82, 0x1285196969a0c079,
    0x2bb9c73591f544fb, 0x413ad7a94588be28, 0xe2b9dfa038807d18, 0x24e25f44ee1639e3, 0x33e6e8965807a894, 0xb7a68b705c5e4673,
    0xf6371889b5007823, 0x6b53aac298a1758c, 0xaa420beb73f09acc, 0x184e44bde5f8b8ba, 0x206dff9e2e890ef8, 0x9c51ba366f443c89,
    0xafee07e4f51e84e4, 0xd4cb621cc84e7e17, 0x45d3716ca0e46566, 0x1bdc68c72ff366c6, 0x3dc3cb59104ea82c, 0xd4afaa5fe015f0a2,
    0x08a552cf2da2b46a, 0x7a5e25ab6f18b9c4, 0x0531eef446fe556d, 0x1162d33a581e622f, 0xbff630abec73b189, 0x58e34710fb896fdf,
    0xbc11f1ee7c8b34a6, 0x914cc8ddc6718a74, 0x33ca89d4bd32e5de, 0x62289b78189341b1, 0x7d4f96c01f6dfd86, 0x103d2de1bb3239dd,
    0x4a9da385b1948bbe, 0x38114dc8904e2b87, 0xcf0c5db159581b7c, 0x7ced016125ebdd7f, 0x00c9ec7d58443ea6, 0x8e1d6d133326a703,
    0xeb61bdfb55aac5ce, 0xb46b905820f53b30, 0x77ec18cdd1cd8aaa, 0x9ae921fe5045708b, 0xd1ddfa303489ab17, 0x129b333e6795199a,
    0x7e99e8951b63df82, 0x596315c298109ea8, 0xb2508217459d17b4, 0x859812cc33079b3c, 0x6c395e3cd58d11b2, 0xfa02a4ab875a5560,
    0x4bdbde663bbe7cc9, 0x3c4bc55523f309ad, 0xda50a2246fe77f7f, 0x12e41c41fc699d34, 0x9d36833b865afadb, 0x7c70cf56810e2487,
    0x5f2e5448c7e1db40, 0x935dba9ac375e6b8, 0xe50bddfa9666ae46, 0x0286c3404f5a872c, 0x3cb559c257470a52, 0x59e11071919b1fa5,
    0x2fc50e5a1cb2e86d, 0x4f35299957c12713, 0x24e1be6df5dddda3, 0xf673311c4a452b78, 0x5cabfcc9e165edc0, 0x09069dc61cf29bdd,
    0xc1d7264959935930, 0xdff0b48f09c02048, 0xecf82df665f246b1, 0x285fdbde6c8d27f6, 0x5c8628fc46a5eb58, 0xc6f0a399e9ec120e,
    0x7fcf5a6da2e34abb, 0xb5ae639fe0aad3ce, 0x6b6a09cd605f93fc, 0xb9d41f3f5043a8e1, 0x6a1508558259c598, 0x1cfbb59d1a7ca022,
    0xc87347a4434e96a1, 0xf95e82d16982f593, 0x94fc506d56072bf4, 0x1c0aefa8f6bbbd51, 0x764f20d812ae1658, 0x5cc5810b7a019304,
    0xd2c9d1f3fe8df13c, 0x9bde4588069ee725, 0xe89785724d8f8d66, 0x1fd5d1a4fcca9ff3, 0x8fec8fddf1883ad4, 0xe19949224b1d6fa1,
    0xa2d6169b62d850d6, 0x527a09387a012705, 0xda8b1b81bd142ee2, 0xd7fff0e753a84b95, 0x71aaf744353c28fe, 0x77a05f8f79f106c1,
    0x8844b6bd81bdeb5d, 0xb7d16d1465a92053, 0x6f0fd42d7936770e, 0x10095c802492c52e, 0x32ed5f7c6db37fb2, 0x824247444670bb41,
    0xedcef59c8aa94e3d, 0xd67594b854b6c4a9, 0xa3b5bc9f8536b158, 0x4d8875f95f74ec35, 0x40ae95e924467ccb, 0x276572d8d3c9ef5f,
    0xcecd46d3f7b7e606, 0xfcfb59df3314fc09, 0x0796f38f7b9e49d7, 0x72fefe0a71ca4565, 0x335b71be16e149cd, 0x6c429fe882d260b1,
    0x37537f1e6a42326f, 0xa54c466e5db96a85, 0xe402b98e26ed7008, 0xcd7bab28019ff2e9, 0xc1b166f34a67450c, 0x77056cfbf831f334,
    0x0c250b6bd6664d7f, 0xa5923815ee4f3876, 0x83821c9c63e645b8, 0xaa57fa0e85e9113d, 0xd53f42575864f7ae, 0x318ca53b42583307,
    0xe270580326d06c4e, 0x816e33cbbb2bcd79, 0x53a4e6c83edf3537, 0xec7c8dc428f2482c, 0xdfebbeb180ff71d2, 0xc92401aa51f5e87b,
    0x1859f5024cadceaa, 0xf3307dd518b81b7b, 0x38fdc1defc462c80, 0x810d2fd02b491f2a, 0xc61fb2b319e5febd, 0x34d1d6418aeb2578,
    0x8cbd319efd10656f, 0xd3dc76b0d00cc1b6, 0x1c697617a4a5d854, 0x15828e6f6e2a1299, 0xd52f135c4e6aeea0, 0x161a2d5a6fd8bf29,
    0x991d064ca86b000e, 0x79a0169564f05393, 0x0d61d30dc376bb0e, 0x575d1bbec91550f9, 0x3453f646d374cece, 0xbcccef11beef7f30,
    0x744ab48e2f351459, 0xd7f19f5b1228f308, 0xfef029880216f38e, 0x7e29da0bb00e83dd, 0xa0f0602ca7efd42e, 0x9d9071951580e43b,
    0xb04a60af0af07ea9, 0x72dc9cadbcb081fa, 0xbf223d90236d759d, 0xfd494856cdc7b19f, 0x93ab676ef87a2080, 0xf83b12ace154d355,
    0xe1b81ac910b77093, 0x774ab091b4cbeebe, 0x30c0ae79085cd265, 0xd17699e97d1fce3e, 0x3feb203bd28d7f2e, 0x9adfd81576ade7f8,
    0x6c58d367a7eb4e34, 0x6119f91efe0c3de4, 0x6d5e1900fb0aa649, 0x9e7bc9ee19d5b789, 0x08cce53a9147e1b8, 0x4a2789780860bac6,
    0xe685e68cf55cc3fc, 0xf7b9d9e7ee14e182, 0x1dbfe3cb56fec5cd, 0xd295f95112eaef6d, 0xafa3c85ce80f8801, 0x8c987ad1279f07ff,
    0x598384fe32d5b4f5, 0x6dec90596cbbd00b, 0x260c6b7611b4b59b, 0x37ebd0b411341c86, 0x9c103247c56f0e41, 0x14de39948ced57a2,
    0x877e991707f57bae, 0x3f52868e03d1b6f1, 0xa6e1ce2622c71f5b, 0x53e37cca0afcccac, 0xf4ed4a1422c206a3, 0x2cb3a4613868790a,
    0x9887cafd5ce1fe18, 0x8bbed8f6bee2258a, 0x7e474fa402c9f501, 0xe059f501575b134a, 0xd26488c57047df0d, 0xd3309ef4af8409b9,
    0x5f8c71be8ef05c7c, 0x2895fc7ceca99785, 0x72740e79757f9879, 0xb81137761a2d7b0a, 0x2ef05e5be7c69f99, 0xc599345f1fb56385,
    0x5d78d741e7e5cbc6, 0xa53f767895a3fbff, 0xa688f9daa7335a02, 0x69e2b2179aab9995, 0xc61355ea21454f11, 0xccd71c536f0fc7aa,
    0x4a0a9f73c6701b94, 0x1f602a270bb4dd5c, 0x6577cd1ed01d43ee, 0x996ddd33c1414146, 0xbf0561b7d6ad9612, 0x3075f4c73eb6748b,
    0x8fd3d672d9ad9f06, 0x34a6cd38e87841a2, 0x770cda955dc08491, 0x5e97fb498b1e8797, 0x19ef4b0f283e53cb, 0xfc0677f91d847b40,
    0x5dcd0e935c902add, 0xb61f014da11b93b2, 0x43189f931fb501c8, 0x4738fe6be394a9b9, 0xfbdb071c16371e43, 0x2a5053f713c7e358,
    0x5bee0d828ebc4899, 0x7fc5e00d35641a39, 0x3b1f81be4d59ff1d, 0x693c49d341225283, 0x111bdd7024706454, 0xfcf811bdda0187c7,
    0xb027c84298c127be, 0x2d4a9c8d680981b0, 0xc6b65e144d526f97, 0x5de956adabcd5ad1, 0x6ed0a80944374c56, 0xd4ac9b75dadd55f7,
    0x9f314220f5476549, 0x9a02f756a0b46f64, 0xf434bbfaf6d5cf2c, 0x8be48c7ae01181e0, 0xcfbb8fc1feb539e9, 0x12f0c5a0d9136c02,
    0x901e3d63ec6a224f, 0xf93b035c1beddff4, 0xe0e1705c538c6b04, 0x7441d9b6453fff4f, 0x0eda1783ef43986b, 0xc620d0260d3db46c,
    0x50ef948d14e25bfd, 0xcca180e8d3cc76a4, 0xa83ff828a3a5b54c, 0xcd03f90f2d918145, 0xd5a69b33dcc332ba, 0xb6b1fed89a52fd4c,
    0x53b62aeafa0c2547, 0x5f4234206e66ed7b, 0x24e6fd40f7762ce0, 0xf69b31d058f360d6, 0xf51cbd6568cbcd35, 0x6d0f93df8d193a93,
    0xbb32dad5853c3d28, 0x9c5b253a7dc95097, 0x5917d4004a97863a, 0x0f3b7195999ae91b, 0x94025d89f894a025, 0x3e18f2e72b996380,
    0xd653adfa1028a144, 0x032b695b08dfbd4e, 0xdf290288de762f42, 0xa40e385f9e1804fa, 0xfc12600fb711560a, 0x584c05a3b0355250,
    0xa84a4a0a510f88ab, 0xd1ac4b877e4696e4, 0xb95bd67e18321358, 0x2fb17d493171c705, 0x4f5ea86009b5be6a, 0x60fb8a0a061e0118,
    0x277646c5eb4be657, 0xd607a405280f3be6, 0x251bf7da6277137b, 0x9e482d9a27c7a170, 0x4a5391a4de153ec8, 0xcb5e1648116a445a,
    0x95418bf7b2759bf7, 0xbe8b2827b4ef2a3d, 0x7c0db98a7ac6eafd, 0xac080a6ff6f0d5a6, 0x78b49a118f76271c, 0x475d9c904c7003b8,
    0x04ccc1879eb83af6, 0x1620aac495423e7e, 0x154502cbf3eaa265, 0x320acec449031c2d, 0x9c2b8ad087ac3683, 0xc09bb2c43b53ef5b,
    0x76c3f6d1bb902a57, 0x1e63873550db2540, 0xa44d878c822a56cb, 0x30eb70bfc1477fc3, 0xaae65612c9ee9076, 0xe7eaea04043c28eb,
    0x2af5523799412c1c, 0x650ea47f01808d2f, 0x10587dbb716a39eb, 0xcbcfaedcab7a46fd, 0x8dad0c854912c164, 0xbaf7573eaa177caa,
    0x818da97692130e21, 0x27dc36eb8cf43fec, 0xaa20119f4c666099, 0xe2a12323b2aa6e5d, 0x182d9dc931f0a566, 0x6e6dd60232f5e576,
    0xba97946fbfba1e38, 0x35937bd28ebe76e9, 0x28a915f1d9d58d6c, 0xf96fcb0c105cc10b, 0x5b16dadb7e6cc964, 0x25dce87c549a41e9,
    0x584d5722d02ff4ce, 0x6b0698a2aa74e257, 0x46f2a6e7c0a18ee5, 0x24734f106098b0fb, 0xd71bfae32fafec1c, 0x5ce7f8225994960f,
    0xccada96d539c4b08, 0x9f359f5042f13b1c, 0x2f7edf4f215ce4e1, 0x4900a0fe1c44d92d, 0xb2f1895a4c9a55d1, 0xab5db8b7f8fda59a,
    0x505d3dff750f8744, 0x4bca670804b56e63, 0x6ba15a764dbabae6, 0x2a394420ad317648, 0x11452f647ee77c26, 0xeb4517ab725d213b,
    0xcb2047e539d3cb8c, 0x1cd2920a1e895b73, 0x627fd7b2eabe5da4, 0xb988bafd5846aac0, 0x39ff916766c75e9f, 0xefab220ce48e2166,
    0x3ecf591c86acd6db, 0x97bd33422b355f6e, 0x2fc28c4735addc49, 0xb70596182b03f7ec, 0x639240b36307988b, 0x86569e83bab266cb,
    0x82f63e517458bc42, 0x1585552efb469be5, 0x215344ea9c3ebc19, 0x295f17b93c5c5071, 0xef195e8a953e749c, 0x3dea0cfc2487d580,
    0x211000128fff24f4, 0xfef664a4f694dcbc, 0xaff4d9977eb5d0bc, 0xdad28c1615e5ba24, 0xd7ef3078a8dd0d9f, 0x77253041eaad7ecb,
    0x10903ca1eec485bd, 0x5fc89bc729fabe49, 0xad14c7aaceee59a3, 0x3b9890e976a6171d, 0xbdbdf6d3f18d914a, 0x5e34ef48429e08cf,
};
constexpr std::uint64_t castling_hash_values[16] =
{
    0xb4566a5d395db94c, 0xf25c6f77e31ecf94, 0x7ffd385a1e918e47, 0xca9b42d79ede6fd7, 0x204cb560048e0cb4, 0xc6a6e14051d45deb,
    0x593d7f00f7da6689, 0xddbf9f9db2c82de7, 0x390559721e5d5bcc, 0xdc23d46cc5faf6a0, 0xc13a9f9161034413, 0x5404f90aafaf6b05,
    0xbffc7304dbffc888, 0x06afa28043a777ab, 0xa26c3d91f974eadc, 0x0e757fae49d52402,
};
constexpr std::uint64_t color_hash_values[2] =
{
    0x4cbf67ceecb3996c, 0x8cdc4ed1d4881ab5,
};

enum Color : std::uint8_t { WHITE, BLACK };

constexpr inline Color operator~(Color c) { return static_cast<Color>(c ^ 1); }
constexpr inline int rank_fwd(Color c) { return c == WHITE ? 8 : -8; }

enum PieceType : std::uint8_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

enum Piece : std::uint8_t
{
    NONE,
    WPAWN = 1 << 3, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING,
    BPAWN = 1 << 4, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING,
};

constexpr inline Color color(Piece p) { return static_cast<Color>(p >> 4); }
constexpr inline PieceType type(Piece p) { return static_cast<PieceType>(p & 7); }
constexpr inline Piece piece(Color c, PieceType t) { return static_cast<Piece>((c == WHITE ? 1 << 3 : 1 << 4) | t); }

enum Castling : std::uint8_t { WQ = 1 << 0, WK = 1 << 1, BQ = 1 << 2, BK = 1 << 3 };

inline Castling& operator|=(Castling& lhs, Castling rhs) { return lhs = static_cast<Castling>(lhs | rhs); }
inline Castling& operator&=(Castling& lhs, Castling rhs) { return lhs = static_cast<Castling>(lhs & rhs); }
inline Castling operator~(Castling c) { return static_cast<Castling>(~static_cast<std::uint8_t>(c)); }

std::ostream& operator<<(std::ostream& out, Castling c)
{
    if (c)
    {
        if (c & WK)
            out << 'K';
        if (c & WQ)
            out << 'Q';
        if (c & BK)
            out << 'k';
        if (c & BQ)
            out << 'q';
    }
    else
        out << '-';
    return out;
}

enum Square : std::uint8_t { A1 = 0, H1 = 56, H8 = 63, NO_SQUARE = 0xff };

inline Square& operator+=(Square& lhs, int rhs) { return lhs = static_cast<Square>(lhs + rhs); }
inline Square& operator-=(Square& lhs, int rhs) { return lhs = static_cast<Square>(lhs - rhs); }
inline Square operator++(Square& lhs, int)
{
    Square r = lhs;
    lhs = static_cast<Square>(lhs + 1);
    return r;
}

inline Square square(int file, int rank)
{
    return static_cast<Square>(file + (rank << 3));
}

Square parse_square(std::string_view s)
{
    return square(s[0] - 'a', s[1] - '1');
}

std::ostream& operator<<(std::ostream& out, Square sq)
{
    if (sq == NO_SQUARE)
        out << '-';
    else
        out << static_cast<char>('a' + (sq & 7)) << static_cast<char>('1' + (sq >> 3));
    return out;
}

enum BitBoard : std::uint64_t
{
    EMPTY = 0, FILE_A = 0x0101010101010101ULL, FILE_B = FILE_A << 1, FILE_G = FILE_A << 6, FILE_H = FILE_A << 7,
    RANK_1 = 0x00000000000000ffULL, RANK_2 = RANK_1 << 8, RANK_7 = RANK_1 << 48, RANK_8 = RANK_1 << 56,
    QUEEN_SIDE = 0x0f0f0f0f0f0f0f0f, KING_SIDE = ~QUEEN_SIDE,
    LIGHT_SQUARES = 0x55aa55aa55aa55aaULL, DARK_SQUARES = ~LIGHT_SQUARES, ALL = ~0ULL
};

inline BitBoard bb(Square s) { return static_cast<BitBoard>(1ULL << s); }
inline BitBoard& operator|=(BitBoard& lhs, Square rhs) { return lhs = static_cast<BitBoard>(lhs | bb(rhs)); }
inline BitBoard& operator|=(BitBoard& lhs, BitBoard rhs) { return lhs = static_cast<BitBoard>(lhs | rhs); }
inline BitBoard& operator&=(BitBoard& lhs, BitBoard rhs) { return lhs = static_cast<BitBoard>(lhs & rhs); }
inline BitBoard operator|(BitBoard lhs, BitBoard rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) | rhs); }
inline BitBoard operator&(BitBoard lhs, Square rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) & bb(rhs)); }
inline BitBoard operator&(BitBoard lhs, BitBoard rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) & rhs); }
inline BitBoard operator<<(BitBoard lhs, int rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) << rhs); }
inline BitBoard operator>>(BitBoard lhs, int rhs) { return static_cast<BitBoard>(static_cast<uint64_t>(lhs) >> rhs); }
inline BitBoard operator~(BitBoard b) { return static_cast<BitBoard>(~static_cast<std::uint64_t>(b)); }
inline BitBoard operator~(Square sq) { return ~bb(sq); }

inline Square first_square(BitBoard b)
{
    return static_cast<Square>(__builtin_ctzll(b));
}

inline Square pop(BitBoard& b)
{
    Square ret = first_square(b);
    b &= static_cast<BitBoard>(b - 1);
    return ret;
}

inline int popcount(BitBoard b) { return __builtin_popcountll(b); }

template<int Offset>
inline BitBoard shift_signed(BitBoard b)
{
    if constexpr (Offset >= 0)
        return b << Offset;
    else
        return b >> -Offset;
}

template<int Offset>
inline BitBoard smear(BitBoard b)
{
    b |= shift_signed<Offset>(b);
    b |= shift_signed<Offset * 2>(b);
    b |= shift_signed<Offset * 4>(b);
    return b;
}

enum MoveType : std::uint8_t { EN_PASSANT = 1, CASTLING = 2, PROMOTION = 4, CAPTURE = 8, INVALID_TYPE = 15 };

inline MoveType& operator|=(MoveType& lhs, MoveType rhs) { return lhs = static_cast<MoveType>(lhs | rhs); }
inline MoveType& operator&=(MoveType& lhs, MoveType rhs) { return lhs = static_cast<MoveType>(lhs & rhs); }
inline MoveType operator|(MoveType lhs, MoveType rhs) { return lhs |= rhs; }
inline MoveType operator&(MoveType lhs, MoveType rhs) { return lhs &= rhs; }
inline MoveType operator~(MoveType m) { return static_cast<MoveType>(~static_cast<unsigned>(m)); }
inline PieceType promotion(MoveType m) { return static_cast<PieceType>(1 + (m & 3)); }
inline MoveType promotion_move(PieceType t) { return static_cast<MoveType>((t - 1) | PROMOTION); }

enum Move : std::uint16_t { NULL_MOVE = 0, NO_MOVE = 0xfff, FROM_TO_MASK = 0xfff, FROM_TO_SIZE = 0x1000, INVALID = 0xffff };

inline Move move(Square from, Square to, MoveType type) { return static_cast<Move>(from | to << 6 | type << 12); }
inline Square from(Move mv) { return static_cast<Square>(mv & 63); }
inline Square to(Move mv) { return static_cast<Square>((mv >> 6) & 63); }
inline MoveType type(Move mv) { return static_cast<MoveType>((mv >> 12) & 15); }

std::ostream& operator<<(std::ostream& out, Move mv)
{
    if (mv == NO_MOVE)
        out << "(none)";
    else
    {
        out << from(mv) << to(mv);
        if (type(mv) & PROMOTION)
        {
            switch (promotion(type(mv)))
            {
                case QUEEN: out << 'q'; break;
                case ROOK: out << 'r'; break;
                case BISHOP: out << 'b'; break;
                case KNIGHT: out << 'n'; break;
                default: break;
            }
        }
    }
    return out;
}

struct alignas(4) Score
{
    std::int16_t mid;
    std::int16_t end;

    constexpr Score() : mid{}, end{} { }
    constexpr Score(std::int16_t m, std::int16_t e) : mid{m}, end{e} { }
};

inline Score operator-(Score s)
{
    return {static_cast<std::int16_t>(-s.mid), static_cast<std::int16_t>(-s.end)};
}

inline Score& operator+=(Score& lhs, Score rhs)
{
    lhs.mid += rhs.mid;
    lhs.end += rhs.end;
    return lhs;
}

inline Score operator+(Score lhs, Score rhs)
{
    return lhs += rhs;
}

inline Score& operator-=(Score& lhs, Score rhs)
{
    lhs.mid -= rhs.mid;
    lhs.end -= rhs.end;
    return lhs;
}

inline Score operator-(Score lhs, Score rhs)
{
    return lhs -= rhs;
}

inline Score operator*(Score lhs, int rhs)
{
    return {static_cast<std::int16_t>(lhs.mid * rhs), static_cast<std::int16_t>(lhs.end * rhs)};
}

std::ostream& operator<<(std::ostream& out, Score s)
{
    return out << '{' << s.mid << ", " << s.end << '}';
}

#ifndef TUNE
constexpr
#endif
Score material[6] = {{100, 131}, {368, 426}, {376, 473}, {448, 798}, {1077, 1456}, {2000, 2000}};

alignas(64)
#ifndef TUNE
constexpr
#endif
Score piece_square_table[6][64] =
{
    {
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
        {-38, 14}, {-27, 22}, {-36, 23}, {-35, 17}, {-35, 26}, {-5, 14}, {2, 8}, {-39, -2},
        {-37, 1}, {-29, 5}, {-33, -1}, {-25, -10}, {-11, -18}, {-20, 0}, {5, -15}, {-36, -10},
        {-32, 7}, {-22, 9}, {0, -24}, {11, -46}, {28, -42}, {13, -22}, {10, -5}, {-26, -8},
        {-28, 69}, {-4, 49}, {-9, 29}, {19, 1}, {30, -1}, {20, 12}, {1, 44}, {-5, 44},
        {11, 141}, {19, 140}, {29, 110}, {36, 78}, {30, 86}, {69, 90}, {43, 132}, {13, 127},
        {235, 215}, {148, 256}, {108, 257}, {103, 244}, {38, 259}, {22, 271}, {22, 288}, {109, 259},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    },
    {
        {-49, -99}, {-24, -86}, {-19, -58}, {-14, -45}, {-12, -43}, {-17, -58}, {-24, -77}, {-48, -88},
        {-32, -94}, {-23, -32}, {-15, -20}, {5, -11}, {3, -12}, {7, -24}, {-14, -33}, {-30, -55},
        {-28, -62}, {-5, -18}, {11, 19}, {28, 27}, {33, 26}, {19, 15}, {8, -5}, {-19, -45},
        {-3, -37}, {14, 6}, {40, 34}, {46, 43}, {46, 42}, {57, 34}, {35, 6}, {9, -20},
        {0, -14}, {25, 1}, {53, 36}, {75, 36}, {58, 48}, {60, 37}, {37, 21}, {24, -21},
        {-28, -6}, {33, -8}, {64, 19}, {89, 21}, {78, 21}, {110, 4}, {38, -2}, {-24, -12},
        {-44, -32}, {-26, -6}, {35, -6}, {63, -3}, {45, 1}, {50, -18}, {-20, -13}, {-18, -34},
        {-186, -42}, {-102, 7}, {-97, 25}, {-42, 6}, {-27, -7}, {-187, 53}, {-111, 5}, {-114, -87},
    },
    {
        {8, -71}, {8, -62}, {-3, -35}, {-26, -21}, {-20, -32}, {-19, -34}, {-11, -58}, {-7, -62},
        {13, -50}, {19, -38}, {13, -33}, {-3, -14}, {4, -23}, {25, -45}, {18, -33}, {-1, -66},
        {2, -38}, {17, -9}, {14, 11}, {14, 10}, {11, 15}, {15, -2}, {19, -19}, {-1, -44},
        {-11, -34}, {3, -4}, {11, 19}, {26, 26}, {22, 21}, {12, 8}, {2, -11}, {8, -40},
        {-19, -20}, {3, 4}, {10, 12}, {45, 16}, {32, 21}, {19, 15}, {18, -2}, {-13, -20},
        {-34, -1}, {5, 5}, {-5, 22}, {24, 2}, {24, 10}, {39, 19}, {17, 8}, {-18, -5},
        {-74, 12}, {-53, 23}, {-24, 7}, {-53, 23}, {-59, 32}, {-37, 18}, {-64, 29}, {-67, -2},
        {-65, 16}, {-69, 20}, {-125, 39}, {-98, 33}, {-108, 35}, {-181, 62}, {-85, 23}, {-43, 3},
    },
    {
        {-14, -24}, {-19, -25}, {-13, -23}, {-9, -23}, {-2, -34}, {-2, -27}, {-23, -27}, {-28, -37},
        {-37, -35}, {-45, -25}, {-18, -29}, {-17, -28}, {-11, -34}, {7, -48}, {-12, -55}, {-93, -27},
        {-49, -12}, {-45, -6}, {-47, 0}, {-41, -6}, {-36, -8}, {-37, -11}, {-17, -29}, {-43, -29},
        {-53, 15}, {-59, 24}, {-45, 22}, {-46, 20}, {-43, 14}, {-34, 12}, {-38, 7}, {-36, -6},
        {-33, 22}, {-35, 26}, {-15, 20}, {-12, 18}, {-16, 18}, {-9, 17}, {-24, 20}, {-25, 8},
        {-10, 23}, {24, 7}, {26, 8}, {27, 10}, {35, 6}, {44, 5}, {31, 8}, {-6, 16},
        {25, 13}, {18, 17}, {67, 5}, {85, -3}, {53, 8}, {71, -5}, {28, 13}, {27, 9},
        {76, -14}, {66, -5}, {68, -5}, {64, -4}, {29, 10}, {19, 15}, {51, 2}, {59, -1},
    },
    {
        {-17, -94}, {-7, -118}, {-2, -127}, {1, -110}, {-8, -135}, {-16, -179}, {-10, -166}, {-13, -134},
        {-12, -107}, {-13, -86}, {5, -89}, {-1, -79}, {2, -92}, {20, -124}, {24, -161}, {-27, -102},
        {-18, -74}, {-9, -65}, {-24, -27}, {-23, -34}, {-23, -24}, {-10, -39}, {8, -56}, {-15, -71},
        {-18, -70}, {-40, -17}, {-37, -1}, {-41, 28}, {-35, 16}, {-26, 13}, {-5, -23}, {-24, -12},
        {-30, -40}, {-43, -1}, {-45, 24}, {-40, 43}, {-39, 61}, {-29, 46}, {-16, 33}, {-9, 2},
        {-29, -41}, {-38, -3}, {-41, 23}, {-23, 41}, {-17, 63}, {14, 63}, {-2, 45}, {-15, 42},
        {-64, -9}, {-71, 15}, {-32, 30}, {-33, 46}, {-50, 71}, {-6, 48}, {-35, 44}, {-14, 14},
        {-26, -18}, {24, -19}, {38, -4}, {34, 10}, {21, 22}, {79, -12}, {64, -13}, {35, -8},
    },
    {
        {-32, -31}, {36, -33}, {18, -40}, {-67, 3}, {-27, -27}, {-19, -22}, {45, -37}, {56, -79},
        {54, -53}, {8, -7}, {-27, 12}, {-57, 21}, {-50, 24}, {-16, 8}, {10, -4}, {40, -42},
        {-53, -4}, {-10, 9}, {-40, 24}, {-32, 29}, {-31, 28}, {-43, 21}, {-31, 12}, {-82, 9},
        {-38, 3}, {29, 4}, {5, 25}, {22, 28}, {-9, 33}, {-25, 24}, {-16, 11}, {-118, 18},
        {32, -10}, {102, -6}, {160, 1}, {163, 12}, {128, 17}, {85, 18}, {39, 9}, {-40, 3},
        {60, 8}, {212, -13}, {281, -8}, {282, -4}, {265, 4}, {254, 2}, {129, 19}, {63, 9},
        {53, -19}, {214, -28}, {266, -21}, {273, -22}, {247, -13}, {196, -3}, {118, 5}, {46, -22},
        {131, -80}, {264, -54}, {215, -23}, {191, -13}, {143, 10}, {207, -12}, {95, -15}, {-80, -24},
    },
};

Score piece_square_value(Square sq, Color c, PieceType t)
{
    unsigned flip = c == WHITE ? 0 : 56;
    return material[t] + piece_square_table[t][sq ^ flip];
}

alignas(64) static BitBoard knight_attack[64];
alignas(64) static BitBoard king_attack[64];
alignas(64) static BitBoard pawn_attack[2][64];
alignas(64) static BitBoard pawn_push[2][64];
alignas(64) static BitBoard pawn_double_push[2][64];
alignas(64) static BitBoard ray[64][8];
alignas(64) static BitBoard king_threat[64];

struct LineMask
{
    BitBoard full;
    BitBoard lower;
    BitBoard upper;

    LineMask() : full{}, lower{}, upper{} { }
    LineMask(BitBoard l, BitBoard u) : full{l | u}, lower{l}, upper{u} { }
};

alignas(64) static LineMask line_masks[64][4];

BitBoard line_attack(BitBoard blockers, const LineMask& lm)
{
    BitBoard lower = lm.lower & blockers;
    BitBoard upper = lm.upper & blockers;
    return static_cast<BitBoard>(lm.full & (upper ^ (upper - (0x8000000000000000ULL >> __builtin_clzll(lower | 1)))));
}

BitBoard bishop_attack_comp(Square sq, BitBoard blockers)
{
    return line_attack(blockers, line_masks[sq][0])
        | line_attack(blockers, line_masks[sq][1]);
}

BitBoard rook_attack_comp(Square sq, BitBoard blockers)
{
    return line_attack(blockers, line_masks[sq][2])
        | line_attack(blockers, line_masks[sq][3]);
}

#ifdef __BMI2__
alignas(64) static BitBoard bishop_pext_masks[64];
alignas(64) static BitBoard bishop_pext_table[64][1 << 9];

alignas(64) static BitBoard rook_pext_masks[64];
alignas(64) static BitBoard rook_pext_table[64][1 << 12];

BitBoard bishop_attack(Square sq, BitBoard blockers)
{
    return bishop_pext_table[sq][_pext_u64(blockers, bishop_pext_masks[sq])];
}

BitBoard rook_attack(Square sq, BitBoard blockers)
{
    return rook_pext_table[sq][_pext_u64(blockers, rook_pext_masks[sq])];
}
#else
inline BitBoard bishop_attack(Square sq, BitBoard blockers)
{
    return bishop_attack_comp(sq, blockers);
}

inline BitBoard rook_attack(Square sq, BitBoard blockers)
{
    return rook_attack_comp(sq, blockers);
}
#endif

BitBoard queen_attack(Square sq, BitBoard blockers)
{
    return bishop_attack(sq, blockers) | rook_attack(sq, blockers);
}

BitBoard offset_bitboard(int file, int rank, const std::pair<int, int> (&offsets)[8])
{
    BitBoard ret{};
    for (auto off : offsets)
    {
        if (file + off.first >= 0 && file + off.first <= 7 && rank + off.second >= 0 && rank + off.second <= 7)
            ret |= square(file + off.first, rank + off.second);
    }
    return ret;
}

void init_bitboards()
{
    constexpr std::pair<int, int> knight_offsets[8] = {{-2, -1}, {-1, -2}, {2, -1}, {-1, 2}, {-2, 1}, {1, -2}, {2, 1}, {1, 2}};
    constexpr std::pair<int, int> king_offsets[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    constexpr std::pair<int, int> ray_offsets[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}};

    for (Square sq = A1; sq <= H8; sq++)
    {
        int file = sq & 7;
        int rank = sq >> 3;

        knight_attack[sq] = offset_bitboard(file, rank, knight_offsets);
        king_attack[sq] = offset_bitboard(file, rank, king_offsets);

        king_threat[sq] = king_attack[sq];
        if (file > 1)
            king_threat[sq] |= king_threat[sq] >> 1;
        if (file < 6)
            king_threat[sq] |= king_threat[sq] << 1;
        if (rank > 1)
            king_threat[sq] |= king_threat[sq] >> 8;
        if (rank < 6)
            king_threat[sq] |= king_threat[sq] << 8;
        king_threat[sq] &= ~bb(sq);

        for (Color c : {WHITE, BLACK})
        {
            if (rank == (c == WHITE ? 7 : 0))
                continue;
            if (file > 0)
                pawn_attack[c][sq] |= static_cast<Square>(sq - 1 + rank_fwd(c));
            if (file < 7)
                pawn_attack[c][sq] |= static_cast<Square>(sq + 1 + rank_fwd(c));

            pawn_push[c][sq] = bb(static_cast<Square>(sq + rank_fwd(c)));

            if (rank == (c == WHITE ? 1 : 6))
                pawn_double_push[c][sq] = bb(static_cast<Square>(sq + 2 * rank_fwd(c)));
        }

        for (int i = 0; i < 8; ++i)
        {
            auto off = ray_offsets[i];
            int f = file + off.first;
            int r = rank + off.second;
            while (f >= 0 && f <= 7 && r >= 0 && r <= 7)
            {
                ray[sq][i] |= square(f, r);
                f += off.first;
                r += off.second;
            }
        }

        line_masks[sq][0] = {ray[sq][0], ray[sq][4]};
        line_masks[sq][1] = {ray[sq][6], ray[sq][2]};
        line_masks[sq][2] = {ray[sq][1], ray[sq][5]};
        line_masks[sq][3] = {ray[sq][7], ray[sq][3]};

#ifdef __BMI2__
        bishop_pext_masks[sq] = ray[sq][0] | ray[sq][2] | ray[sq][4] | ray[sq][6];
        bishop_pext_masks[sq] &= ~(FILE_A | FILE_H | RANK_1 | RANK_8);
        for (int i = 0; i < 1 << popcount(bishop_pext_masks[sq]); i++)
            bishop_pext_table[sq][i] = bishop_attack_comp(sq, static_cast<BitBoard>(_pdep_u64(i, bishop_pext_masks[sq])));

        rook_pext_masks[sq] = ray[sq][1] | ray[sq][3] | ray[sq][5] | ray[sq][7];
        if (!(bb(sq) & FILE_A))
            rook_pext_masks[sq] &= ~FILE_A;
        if (!(bb(sq) & FILE_H))
            rook_pext_masks[sq] &= ~FILE_H;
        if (!(bb(sq) & RANK_1))
            rook_pext_masks[sq] &= ~RANK_1;
        if (!(bb(sq) & RANK_8))
            rook_pext_masks[sq] &= ~RANK_8;
        for (int i = 0; i < 1 << popcount(rook_pext_masks[sq]); i++)
            rook_pext_table[sq][i] = rook_attack_comp(sq, static_cast<BitBoard>(_pdep_u64(i, rook_pext_masks[sq])));
#endif
    }
}

struct Memo
{
    Piece moved;
    Piece captured;
    Castling castling;
    Square en_passant;
    int halfmove_clock;
};

struct Position
{
    Memo do_move(Move mv);
    void undo_move(Move mv, const Memo& memo);

    void parse(std::istream& fen);
    void reset();
    Move parse_move(std::string_view s) const;

    void debug(std::ostream& out);

    BitBoard all_bb() const { return color_bb[WHITE] | color_bb[BLACK]; }
    std::uint64_t hash() const { return piece_hash ^ castling_hash_values[castling] ^ color_hash_values[next]; }

    void clear(Square sq, Piece p) { clear(sq, color(p), type(p)); }
    void clear(Square sq, Color c, PieceType t);
    void set(Square sq, Piece p) { set(sq, color(p), type(p), p); }
    void set(Square sq, Color c, PieceType t) { set(sq, c, t, piece(c, t)); }
    void set(Square sq, Color c, PieceType t, Piece p);

    Piece squares[64];
    BitBoard color_bb[2];
    BitBoard type_bb[6];
    Color next;
    Castling castling;
    Square en_passant;
    int halfmove_clock;
    int fullmove_counter;
    std::uint64_t piece_hash;
    Score piece_square_values[2];
} position;

void Position::clear(Square sq, Color c, PieceType t)
{
    color_bb[c] &= ~sq;
    type_bb[t] &= ~sq;
    squares[sq] = NONE;
    piece_hash ^= piece_hash_values[sq | 64 * c | 128 * t];
    piece_square_values[c] -= piece_square_value(sq, c, t);
}

void Position::set(Square sq, Color c, PieceType t, Piece p)
{
    assert(t >= PAWN && t <= KING);
    color_bb[c] |= sq;
    type_bb[t] |= sq;
    squares[sq] = p;
    piece_hash ^= piece_hash_values[sq | 64 * c | 128 * t];
    piece_square_values[c] += piece_square_value(sq, c, t);
}

Memo Position::do_move(Move mv)
{
    Memo memo{squares[from(mv)], squares[to(mv)], castling, en_passant, halfmove_clock};
    ++halfmove_clock;

    if (mv)
    {
        MoveType mt = type(mv);
        if (mt & CAPTURE)
        {
            Square cap_sq = to(mv);
            if (mt == (EN_PASSANT | CAPTURE))
            {
                cap_sq = static_cast<Square>(cap_sq + rank_fwd(~next));
                memo.captured = squares[cap_sq];
            }

            clear(cap_sq, memo.captured);
            halfmove_clock = 0;
        }
        Piece moved = squares[from(mv)];
        clear(from(mv), moved);
        if (mt & PROMOTION)
            set(to(mv), next, promotion(mt));
        else
            set(to(mv), moved);
        en_passant = NO_SQUARE;

        if (type(moved) == PAWN)
        {
            halfmove_clock = 0;
            if (mt == EN_PASSANT)
            {
                Square ep = static_cast<Square>(from(mv) + rank_fwd(next));
                if (pawn_attack[next][ep] & type_bb[PAWN] & color_bb[~next])
                    en_passant = ep;
            }
        }
        if (mt == CASTLING)
        {
            int rank = from(mv) & ~7;
            Square rook_from = static_cast<Square>(rank | (to(mv) < from(mv) ? 0 : 7));
            Square rook_to = static_cast<Square>(rank | (to(mv) < from(mv) ? 3 : 5));
            assert(type_bb[ROOK] & rook_from);
            clear(rook_from, next, ROOK);
            set(rook_to, next, ROOK);
            castling &= ~static_cast<Castling>(3 << (2 * next));
        }
        else if (castling)
        {
            if (type(moved) == KING)
                castling &= ~static_cast<Castling>(3 << (2 * next));
            else if (type(moved) == ROOK)
                castling &= ~static_cast<Castling>(((from(mv) & 1) ? WK : WQ) << (2 * next));
            if ((type(mv) & CAPTURE) && type(memo.captured) == ROOK)
                castling &= ~static_cast<Castling>(((to(mv) & 1) ? WK : WQ) << (2 * ~next));
        }
    }
    else
        en_passant = NO_SQUARE;

    next = ~next;
    return memo;
}

void Position::undo_move(Move mv, const Memo& memo)
{
    next = ~next;
    halfmove_clock = memo.halfmove_clock;
    en_passant = memo.en_passant;
    castling = memo.castling;

    if (mv)
    {
        clear(to(mv), squares[to(mv)]);
        set(from(mv), memo.moved);

        MoveType mt = type(mv);
        if (mt & CAPTURE)
        {
            Square cap_sq = to(mv);
            if (mt == (EN_PASSANT | CAPTURE))
                cap_sq = static_cast<Square>(cap_sq + rank_fwd(~next));
            set(cap_sq, memo.captured);
        }

        if (mt == CASTLING)
        {
            int rank = from(mv) & ~7;
            Square rook_from = static_cast<Square>(rank | (to(mv) < from(mv) ? 0 : 7));
            Square rook_to = static_cast<Square>(rank | (to(mv) < from(mv) ? 3 : 5));
            clear(rook_to, next, ROOK);
            set(rook_from, next, ROOK);
        }
    }
}

void Position::parse(std::istream& fen)
{
    reset();

    std::string token;
    fen >> token;

    Square sq = H1;

    for (char ch : token)
    {
        if (ch == '/')
            sq -= 16;
        else if (ch >= '1' && ch <= '8')
            sq += ch - '0';
        else
        {
            Piece p;
            switch (ch)
            {
                case 'P': p = WPAWN; break;
                case 'N': p = WKNIGHT; break;
                case 'B': p = WBISHOP; break;
                case 'R': p = WROOK; break;
                case 'Q': p = WQUEEN; break;
                case 'K': p = WKING; break;
                case 'p': p = BPAWN; break;
                case 'n': p = BKNIGHT; break;
                case 'b': p = BBISHOP; break;
                case 'r': p = BROOK; break;
                case 'q': p = BQUEEN; break;
                case 'k': p = BKING; break;
                default: p = NONE; break;
            }
            set(sq++, p);
        }
    }

    fen >> token;
    if (token == "w")
        next = WHITE;
    else if (token == "b")
        next = BLACK;

    fen >> token;
    castling = Castling{};
    for (char ch : token)
    {
        switch (ch)
        {
            case 'K': castling |= WK; break;
            case 'Q': castling |= WQ; break;
            case 'k': castling |= BK; break;
            case 'q': castling |= BQ; break;
        }
    }

    fen >> token;
    if (token == "-")
        en_passant = NO_SQUARE;
    else
        en_passant = parse_square(token);

    fen >> halfmove_clock >> fullmove_counter;
}

void Position::reset()
{
    piece_hash = 0;
    for (BitBoard& b : color_bb)
        b = EMPTY;
    for (BitBoard& b : type_bb)
        b = EMPTY;
    for (Piece& p : squares)
        p = NONE;
    for (Score& v : piece_square_values)
        v = {};
}

Move Position::parse_move(std::string_view s) const
{
    Square from = parse_square(s.substr(0, 2));
    Square to = parse_square(s.substr(2, 2));

    MoveType mt{};
    if (s.length() > 4)
    {
        PieceType promotion{};
        switch (s[4])
        {
            case 'n': promotion = KNIGHT; break;
            case 'b': promotion = BISHOP; break;
            case 'r': promotion = ROOK; break;
            case 'q': promotion = QUEEN; break;
        }
        mt = promotion_move(promotion);
    }
    else if (type(squares[from]) == PAWN)
    {
        if (to == en_passant)
            mt = EN_PASSANT | CAPTURE;
        else if (std::abs(from - to) == 16)
            mt = EN_PASSANT;
    }
    else if (type(squares[from]) == KING)
    {
        if (std::abs((from & 7) - (to & 7)) > 1)
            mt = CASTLING;
    }

    if (squares[to])
        mt |= CAPTURE;
    return move(from, to, mt);
}

void Position::debug(std::ostream& out)
{
    out << "info string fen ";

    for (int rank = 7; rank >= 0; --rank)
    {
        int blanks = 0;

        for (int file = 0; file < 8; ++file)
        {
            Square sq = square(file, rank);
            if (squares[sq] == NONE)
            {
                ++blanks;
            }
            else
            {
                if (blanks)
                {
                    out << blanks;
                    blanks = 0;
                }
                switch (squares[sq])
                {
                    case WPAWN: out << 'P'; break;
                    case WKNIGHT: out << 'N'; break;
                    case WBISHOP: out << 'B'; break;
                    case WROOK: out << 'R'; break;
                    case WQUEEN: out << 'Q'; break;
                    case WKING: out << 'K'; break;
                    case BPAWN: out << 'p'; break;
                    case BKNIGHT: out << 'n'; break;
                    case BBISHOP: out << 'b'; break;
                    case BROOK: out << 'r'; break;
                    case BQUEEN: out << 'q'; break;
                    case BKING: out << 'k'; break;
                    default: break;
                }
            }
        }

        if (blanks)
        {
            out << blanks;
            blanks = 0;
        }
        if (rank > 0)
            out << '/';
    }

    out << ' ' << (next == WHITE ? 'w' : 'b') << ' ' << castling << ' ' << en_passant << ' ' << halfmove_clock << " 1\n";
}

BitBoard attackers(Square sq, Color c)
{
    return position.color_bb[c] & (
        (pawn_attack[~c][sq] & position.type_bb[PAWN])
        | (knight_attack[sq] & position.type_bb[KNIGHT])
        | (bishop_attack(sq, position.all_bb()) & position.type_bb[BISHOP])
        | (rook_attack(sq, position.all_bb()) & position.type_bb[ROOK])
        | (queen_attack(sq, position.all_bb()) & position.type_bb[QUEEN])
        | (king_attack[sq] & position.type_bb[KING])
    );
}

bool is_check(Move mv, Square king_sq)
{
    switch (type(position.squares[from(mv)]))
    {
        case PAWN:
            return pawn_attack[position.next][to(mv)] & king_sq;
        case KNIGHT:
            return knight_attack[to(mv)] & king_sq;
        case BISHOP:
            return bishop_attack(to(mv), position.all_bb()) & king_sq;
        case ROOK:
            return rook_attack(to(mv), position.all_bb()) & king_sq;
        case QUEEN:
            return (bishop_attack(to(mv), position.all_bb()) | rook_attack(to(mv), position.all_bb())) & king_sq;
        default:
            return false;
    }
}

struct alignas(16) Stack
{
    std::uint64_t key;
    Move killer_moves[2];
    Move prev_move;
    bool in_nmp;
    bool singular;

    void save_killer(Move move);
    bool is_killer(Move move) const { return move == killer_moves[0] || move == killer_moves[1]; }
};

void Stack::save_killer(Move move)
{
    if (move != killer_moves[0])
    {
        killer_moves[1] = killer_moves[0];
        killer_moves[0] = move;
    }
}

struct MoveHistory
{
    std::int16_t value;

    MoveHistory(std::int16_t v = -8192) : value{v} { }

    void hit(int n, bool cut)
    {
        value = static_cast<std::int16_t>(value - n * value / 8192 + (cut ? n : -n));
    }
};

alignas(64) static MoveHistory history[2][FROM_TO_SIZE];
alignas(64) static MoveHistory capture_history[30][64];

constexpr int COUNTER_MOVE_SIZE = 2 * 6 * 64 + 1;
alignas(64) static Move counter_moves[COUNTER_MOVE_SIZE];

int counter_index(Move prev_move)
{
    if (!prev_move)
        return COUNTER_MOVE_SIZE - 1;
    PieceType moved = (type(prev_move) & PROMOTION) ? PAWN : type(position.squares[to(prev_move)]);
    return to(prev_move) + 64 * position.next + 128 * moved;
}

enum MoveGenType { START, BEST, CAPTURES, QUIETS, END };

struct alignas(4) RankedMove
{
    Move move;
    std::int16_t rank;
};

bool operator<(RankedMove lhs, RankedMove rhs)
{
    return lhs.rank > rhs.rank;
}

struct MoveGen
{
    MoveGenType wanted;
    MoveGenType generated;
    RankedMove moves[256];
    int count;
    int index;
    BitBoard checkers;
    Move best_move;
    Move counter_move;
    const Stack& stack;

    explicit MoveGen(MoveGenType w, BitBoard ch, Move best, Move cm, const Stack& st)
        : wanted{w}, generated{best ? START : BEST}, moves{}, count{}, index{}, checkers{ch}, best_move{best}, counter_move{cm}, stack{st} {}

    Move next();

    void generate();
    std::int16_t rank_capture(Move mv) const;
    std::int16_t rank_quiet(Move mv) const;
    template<PieceType Type> void generate_pieces(MoveGenType gen, BitBoard from_mask, BitBoard to_mask);
    template<PieceType Type> void generate_piece(MoveGenType gen, Square sq, BitBoard to_mask);
    void generate_targets(MoveGenType gen, Square sq, BitBoard targets);
    void generate_targets(Square sq, BitBoard targets, MoveType mt);
    void generate_pawn_targets(Square sq, BitBoard targets, MoveType mt);
    void generate_target(Square sq, Square target, MoveType mt);
    template<Color color> void generate_castling_moves(Square sq);
    template<Color color, int Offset> void generate_castling_move(Square sq);
};

void MoveGen::generate_target(Square sq, Square target, MoveType mt)
{
    Move mv = move(sq, target, mt);
    if ((generated == BEST) == (mv == best_move))
        moves[count++] = {mv, (mt & CAPTURE) ? rank_capture(mv) : rank_quiet(mv)};
}

void MoveGen::generate_targets(Square sq, BitBoard targets, MoveType mt)
{
    while (targets)
        generate_target(sq, pop(targets), mt);
}

void MoveGen::generate_pawn_targets(Square sq, BitBoard targets, MoveType mt)
{
    if ((sq >> 3) == (position.next == WHITE ? 6 : 1))
    {
        for (PieceType t : {QUEEN, ROOK, BISHOP, KNIGHT})
            generate_targets(sq, targets, promotion_move(t) | mt);
    }
    else
        generate_targets(sq, targets, mt);
}

void MoveGen::generate_targets(MoveGenType gen, Square sq, BitBoard targets)
{
    if (gen == CAPTURES)
        generate_targets(sq, targets & position.color_bb[~position.next], CAPTURE);
    if (gen == QUIETS)
        generate_targets(sq, targets & ~position.all_bb(), {});
}

template<Color color, int Offset> void MoveGen::generate_castling_move(Square sq)
{
    BitBoard between = static_cast<BitBoard>((Offset < 0 ? 0xeULL : 0x60ULL) << (color == WHITE ? 0 : 56));
    if (between & position.all_bb())
        return;

    if (attackers(static_cast<Square>(sq + Offset), ~position.next))
        return;

    generate_target(sq, static_cast<Square>(sq + 2 * Offset), CASTLING);
}

template<Color color> void MoveGen::generate_castling_moves(Square sq)
{
    if (position.castling & (WQ << (2 * color)))
        generate_castling_move<color, -1>(sq);
    if (position.castling & (WK << (2 * color)))
        generate_castling_move<color, 1>(sq);
}

template<PieceType Type> void MoveGen::generate_piece(MoveGenType gen, Square sq, BitBoard to_mask)
{
    if (Type == PAWN)
    {
        if (gen == CAPTURES)
        {
            generate_pawn_targets(sq, pawn_attack[position.next][sq] & position.color_bb[~position.next] & to_mask, CAPTURE);
            if (position.en_passant != NO_SQUARE && (pawn_attack[position.next][sq] & position.en_passant & to_mask))
                generate_target(sq, position.en_passant, EN_PASSANT | CAPTURE);
        }
        if (gen == QUIETS)
        {
            BitBoard push = pawn_push[position.next][sq] & ~position.all_bb();
            if (push)
            {
                generate_pawn_targets(sq, push & to_mask, {});
                generate_targets(sq, pawn_double_push[position.next][sq] & ~position.all_bb() & to_mask, EN_PASSANT);
            }
        }
    }
    else if (Type == KNIGHT)
        generate_targets(gen, sq, knight_attack[sq] & to_mask);
    else if (Type == BISHOP)
        generate_targets(gen, sq, bishop_attack(sq, position.all_bb()) & to_mask);
    else if (Type == ROOK)
        generate_targets(gen, sq, rook_attack(sq, position.all_bb()) & to_mask);
    else if (Type == QUEEN)
        generate_targets(gen, sq, queen_attack(sq, position.all_bb()) & to_mask);
    else if (Type == KING)
    {
        generate_targets(gen, sq, king_attack[sq] & to_mask);
        if (gen == QUIETS && !checkers && (to_mask & ~king_attack[sq]))
        {
            if (position.next == WHITE)
                generate_castling_moves<WHITE>(sq);
            else
                generate_castling_moves<BLACK>(sq);
        }
    }
}

template<PieceType Type> void MoveGen::generate_pieces(MoveGenType gen, BitBoard from_mask, BitBoard to_mask)
{
    if (!to_mask)
        return;
    BitBoard pieces = position.color_bb[position.next] & position.type_bb[Type] & from_mask;
    while (pieces)
        generate_piece<Type>(gen, pop(pieces), to_mask);
}

std::int16_t MoveGen::rank_capture(Move mv) const
{
    return static_cast<std::int16_t>(7660 * (type(position.squares[to(mv)]) - 3) +
            capture_history[6 * type(position.squares[to(mv)]) + type(position.squares[from(mv)])][to(mv)].value);
}

std::int16_t MoveGen::rank_quiet(Move mv) const
{
    std::int16_t rank = history[position.next][mv & FROM_TO_MASK].value;
    if (stack.is_killer(mv))
        rank += 16354;
    if (mv == counter_move)
        rank += 4523;
    return rank;
}

void MoveGen::generate()
{
    generated = static_cast<MoveGenType>(generated + 1);
    MoveGenType gen = generated;
    BitBoard from_mask = ALL;
    BitBoard to_mask = ALL;
    if (generated == BEST)
    {
        gen = type(best_move) & CAPTURE ? CAPTURES : QUIETS;
        if (gen > wanted)
            return;
        from_mask = bb(from(best_move));
        to_mask = bb(to(best_move));
    }
    BitBoard king_to_mask = to_mask;
    if (generated != BEST && checkers)
    {
        if (gen == CAPTURES)
            to_mask &= checkers | ((checkers & position.type_bb[PAWN]) && position.en_passant ? bb(position.en_passant) : EMPTY);
        else if (checkers & (position.type_bb[PAWN] | position.type_bb[KNIGHT]))
            to_mask = EMPTY;
        else
        {
            Square king_sq = first_square(position.type_bb[KING] & position.color_bb[position.next]);
            if (checkers & king_attack[king_sq])
                to_mask = EMPTY;
            else if (checkers & position.type_bb[BISHOP])
                to_mask &= bishop_attack(king_sq, position.all_bb());
            else if (checkers & position.type_bb[ROOK])
                to_mask &= rook_attack(king_sq, position.all_bb());
            else if (checkers & position.type_bb[QUEEN])
                to_mask &= queen_attack(king_sq, position.all_bb());
        }
    }
    generate_pieces<PAWN>(gen, from_mask, to_mask);
    generate_pieces<KNIGHT>(gen, from_mask, to_mask);
    generate_pieces<BISHOP>(gen, from_mask, to_mask);
    generate_pieces<ROOK>(gen, from_mask, to_mask);
    generate_pieces<QUEEN>(gen, from_mask, to_mask);
    generate_pieces<KING>(gen, from_mask, king_to_mask);
    std::sort(&moves[index], &moves[count]);
}

Move MoveGen::next()
{
    while (index >= count && generated < wanted)
        generate();
    if (index >= count)
        return NULL_MOVE;
    return moves[index++].move;
}

#ifndef TUNE
constexpr
#endif
Score pawn_evals[4][15] =
{
    {
        {-8, -1}, {1, 11}, {-90, 49}, {-74, 56}, {-28, 124}, {-12, -14}, {27, 23}, {45, 25},
        {-90, 58}, {2, 20}, {28, 35}, {-85, 66}, {-121, 88}, {-5, 43}, {-3, 1},
    },
    {
        {0, -1}, {14, -3}, {-16, 26}, {0, 36}, {-9, 93}, {-14, 11}, {28, 8}, {17, 28},
        {-42, 67}, {9, 8}, {1, 49}, {-3, 32}, {-13, 42}, {5, 38}, {-9, 13},
    },
    {
        {1, 3}, {13, -5}, {3, 20}, {7, 24}, {14, 69}, {8, 16}, {26, 19}, {21, 32},
        {-7, 62}, {14, 9}, {28, 30}, {9, 15}, {14, 20}, {11, 36}, {9, 9},
    },
    {
        {1, 6}, {7, 8}, {18, 13}, {12, 4}, {18, 79}, {9, 26}, {18, 47}, {32, 37},
        {9, 49}, {12, 23}, {23, 40}, {21, -15}, {11, 9}, {8, 37}, {8, 11},
    },
};

template<Color C>
Score evaluate_pawns()
{
    constexpr int FWD = C == WHITE ? 8 : -8;
    BitBoard all_pawns = position.type_bb[PAWN];
    if (!all_pawns)
        return Score{0, 0};

    BitBoard own_pawns = all_pawns & position.color_bb[C];
    BitBoard opp_pawns = all_pawns & position.color_bb[~C];
    BitBoard own_attack = shift_signed<FWD - 1>(own_pawns & ~FILE_A) | shift_signed<FWD + 1>(own_pawns & ~FILE_H);
    BitBoard opp_attack = shift_signed<-FWD - 1>(opp_pawns & ~FILE_A) | shift_signed<-FWD + 1>(opp_pawns & ~FILE_H);
    BitBoard adjacent = shift_signed<-1>(own_pawns & ~FILE_A) | shift_signed<1>(own_pawns & ~FILE_H);
    BitBoard own_blocked = shift_signed<-FWD>(own_pawns);
    BitBoard doubled = smear<-FWD>(own_blocked);
    BitBoard opp_blocked = shift_signed<-FWD>(opp_pawns);
    BitBoard backwards = shift_signed<-FWD>(opp_attack) & ~adjacent & ~own_attack;
    constexpr BitBoard SECOND_RANK = C == WHITE ? RANK_2 : RANK_7;
    constexpr BitBoard NEAR_RANKS = static_cast<BitBoard>(C == WHITE ? 0x00000000ffffff00ULL : 0x00ffffff00000000ULL);
    constexpr BitBoard FAR_RANKS = static_cast<BitBoard>(C == WHITE ? 0x00ffffff00000000ULL : 0x00000000ffffff00ULL);
    constexpr BitBoard CP_RANKS1 = static_cast<BitBoard>(C == WHITE ? 0x00000000ffff0000ULL : 0x0000ffff00000000ULL);
    constexpr BitBoard CP_RANKS2 = static_cast<BitBoard>(C == WHITE ? 0x0000ffff00000000ULL : 0x00000000ffff0000ULL);
    constexpr BitBoard PP_RANKS = static_cast<BitBoard>(C == WHITE ? 0x0000ffffff000000ULL : 0x000000ffffff0000ULL);
    constexpr BitBoard BP_RANKS = static_cast<BitBoard>(C == WHITE ? 0x0000000000ffff00ULL : 0x00ffff0000000000ULL);

    BitBoard unmoved_pawns = own_pawns & SECOND_RANK;
    BitBoard moved_pawns = own_pawns & ~SECOND_RANK;

    const Score* pe = pawn_evals[(popcount(all_pawns) - 1) / 4];
    Score r{0, 0};

    if (unmoved_pawns)
    {
        r += pe[0] * popcount(unmoved_pawns)
                + pe[1] * popcount(unmoved_pawns & adjacent)
                - pe[2] * popcount(unmoved_pawns & own_blocked)
                - pe[3] * popcount(unmoved_pawns & doubled)
                + pe[4] * popcount(unmoved_pawns & opp_blocked)
                - pe[5] * popcount(unmoved_pawns & backwards);
    }
    if (moved_pawns)
    {
        r += pe[6] * popcount(moved_pawns & CP_RANKS1 & own_attack)
                + pe[7] * popcount(moved_pawns & CP_RANKS2 & own_attack)
                + pe[8] * popcount(moved_pawns & PP_RANKS & ~smear<-FWD>(opp_pawns | opp_attack))
                + pe[9] * popcount(moved_pawns & NEAR_RANKS & adjacent)
                + pe[10] * popcount(moved_pawns & FAR_RANKS & adjacent)
                - pe[11] * popcount(moved_pawns & own_blocked)
                - pe[12] * popcount(moved_pawns & doubled)
                + pe[13] * popcount(moved_pawns & BP_RANKS & opp_blocked)
                - pe[14] * popcount(moved_pawns & BP_RANKS & backwards);
    }

    return r;
}

struct PawnEvalCache
{
    BitBoard pawns;
    Score value;
};

static constexpr int PAWN_EVAL_CACHE_SIZE_BITS = 7;
static constexpr int PAWN_EVAL_CACHE_SIZE = 1 << PAWN_EVAL_CACHE_SIZE_BITS;

static PawnEvalCache pawn_eval_cache[PAWN_EVAL_CACHE_SIZE];

static constexpr int pawnless_king_table[64] =
{
    -42, -43, -12, -4, -4, -12, -43, -42,
    -43, -4, 4, 6, 6, 4, -4, -43,
    -12, 4, 7, 8, 8, 7, 4, -12,
    -4, 6, 8, 9, 9, 8, 6, -4,
    -4, 6, 8, 9, 9, 8, 6, -4,
    -12, 4, 7, 8, 8, 7, 4, -12,
    -43, -4, 4, 6, 6, 4, -4, -43,
    -42, -43, -12, -4, -4, -12, -43, -42,
};

int evaluate_pawnless(int v)
{
    int piece_count = popcount(position.all_bb());
    if (piece_count <= 4 &&
            !(position.type_bb[PAWN] | position.type_bb[ROOK] | position.type_bb[QUEEN]))
    {
        if (popcount(position.color_bb[WHITE]) <= 2 && popcount(position.color_bb[BLACK]) <= 2)
            return 0;
        if (!position.type_bb[BISHOP])
            return 0;
    }

    Square own_king_sq = first_square(position.type_bb[KING] & position.color_bb[position.next]);
    Square opp_king_sq = first_square(position.type_bb[KING] & position.color_bb[~position.next]);
    int king_value = pawnless_king_table[own_king_sq] - pawnless_king_table[opp_king_sq];

    if (piece_count == 4 && (position.type_bb[ROOK] & position.color_bb[WHITE]) && (position.type_bb[ROOK] & position.color_bb[BLACK]))
    {
        v /= 32;
        v += king_value / 8;
        v += 1;
    }
    else
    {
        if (std::abs(v) < material[PAWN].end)
            v /= 16;
        else if (std::abs(v) < 2 * material[PAWN].end)
            v /= 2;
        v += king_value;
        v += 8;
    }

    return v;
}

template<Color C>
int evaluate_single_pawn(int v)
{
    constexpr int FWD = C == WHITE ? 8 : -8;
    BitBoard own_pawns = position.type_bb[PAWN] & position.color_bb[C];

    if (smear<FWD>(shift_signed<FWD>(own_pawns)) & position.type_bb[KING] & position.color_bb[~C])
        v /= 2;
    return v;
}

#ifndef TUNE
constexpr
#endif
Score mobility_evals[66] =
{
    {-15, -90}, {-12, -8}, {-9, 10}, {-4, 10}, {-1, 9}, {0, 7}, {-1, 3}, {0, -5},
    {2, -16}, {-11, -92}, {-9, -37}, {-2, -33}, {-5, -9}, {-2, 1}, {2, 9}, {3, 11},
    {1, 14}, {-3, 17}, {-3, 10}, {-2, 1}, {4, -6}, {-2, -2}, {6, -16}, {-17, -156},
    {-7, -95}, {-6, -59}, {-6, -23}, {-7, -2}, {-4, 4}, {-3, 8}, {4, 7}, {2, 18},
    {1, 24}, {3, 25}, {4, 25}, {9, 19}, {21, 13}, {32, 1}, {2, -158}, {8, -142},
    {4, -95}, {0, -81}, {1, -70}, {-1, -55}, {-2, -40}, {-3, -27}, {-2, -21}, {-4, -10},
    {-4, 2}, {-4, 6}, {-5, 13}, {-4, 16}, {-5, 18}, {-6, 22}, {-5, 22}, {-7, 22},
    {-3, 17}, {-5, 14}, {-6, 14}, {-2, 11}, {2, 4}, {4, -1}, {-2, -11}, {-9, -12},
    {-18, -9}, {-22, 0},
};

struct Mobility
{
    BitBoard attacks[2][6];
    BitBoard attacks2[2];
};

template<Color C>
Score evaluate_mobility(Mobility& mobility)
{
    constexpr int FWD = C == WHITE ? 8 : -8;

    BitBoard blockers = position.all_bb();
    BitBoard own = position.color_bb[C];
    BitBoard own_pawns = position.type_bb[PAWN] & own;
    mobility.attacks[C][PAWN] = shift_signed<FWD - 1>(own_pawns & ~FILE_A) | shift_signed<FWD + 1>(own_pawns & ~FILE_H);
    BitBoard all_attacks = mobility.attacks[C][PAWN];

    Score r{0, 0};

    BitBoard knights = position.type_bb[KNIGHT] & own;
    while (knights)
    {
        BitBoard m = knight_attack[pop(knights)];
        r += mobility_evals[popcount(m & ~own)];
        mobility.attacks[C][KNIGHT] |= m;
        mobility.attacks2[C] |= (m & all_attacks);
        all_attacks |= m;
    }
    BitBoard bishops = position.type_bb[BISHOP] & own;
    while (bishops)
    {
        BitBoard m = bishop_attack(pop(bishops), blockers & ~(position.type_bb[QUEEN] & own));
        r += mobility_evals[9 + popcount(m & ~own)];
        mobility.attacks[C][BISHOP] |= m;
        mobility.attacks2[C] |= (m & all_attacks);
        all_attacks |= m;
    }
    BitBoard rooks = position.type_bb[ROOK] & own;
    while (rooks)
    {
        BitBoard m = rook_attack(pop(rooks), blockers & ~((position.type_bb[ROOK] | position.type_bb[QUEEN]) & own));
        r += mobility_evals[23 + popcount(m & ~own)];
        mobility.attacks[C][ROOK] |= m;
        mobility.attacks2[C] |= (m & all_attacks);
        all_attacks |= m;
    }
    BitBoard queens = position.type_bb[QUEEN] & own;
    while (queens)
    {
        BitBoard m = queen_attack(pop(queens), blockers);
        r += mobility_evals[38 + popcount(m & ~own)];
        mobility.attacks[C][QUEEN] |= m;
        mobility.attacks2[C] |= (m & all_attacks);
        all_attacks |= m;
    }

    return r;
}

alignas(64)
#ifndef TUNE
constexpr
#endif
Score king_evals[64][4] =
{
    {{-78, 13}, {-25, 10}, {13, -2}, {43, -16}},
    {{-66, -110}, {-23, 129}, {-213, 369}, {-127, 332}},
    {{-23, -8}, {-9, -10}, {7, -15}, {19, -10}},
    {{16, -27}, {-32, 22}, {-70, 70}, {-166, 100}},
    {{-2, 4}, {-4, 12}, {6, 5}, {8, -6}},
    {{11, -35}, {-10, -11}, {-71, 22}, {-99, -9}},
    {{-5, -1}, {-2, -2}, {6, -7}, {14, -19}},
    {{19, -16}, {-20, 7}, {-133, 49}, {-285, 96}},
    {{-36, -8}, {-16, -3}, {8, -6}, {39, 1}},
    {{-66, -112}, {-111, 106}, {-265, 258}, {-168, 248}},
    {{-11, -4}, {-10, -1}, {5, -1}, {20, 4}},
    {{14, -25}, {-25, 16}, {-70, 52}, {-121, 82}},
    {{0, 5}, {-1, 3}, {-1, -6}, {1, -35}},
    {{14, -37}, {0, -11}, {-99, 73}, {-64, 41}},
    {{0, -5}, {1, -9}, {2, -19}, {2, -26}},
    {{23, -12}, {-7, 11}, {-90, 49}, {-227, 101}},
    {{24, -42}, {-20, 4}, {1, 4}, {-1, 13}},
    {{-56, -107}, {-50, -17}, {-202, 124}, {-250, 217}},
    {{-10, -21}, {-12, -4}, {1, 0}, {2, -2}},
    {{22, -25}, {-27, -3}, {-66, 11}, {-43, -29}},
    {{9, 0}, {-12, 14}, {-15, 15}, {3, -38}},
    {{13, -32}, {-29, 4}, {-53, 59}, {-100, -21}},
    {{-9, -9}, {-9, -5}, {-13, 0}, {-29, 19}},
    {{28, -18}, {-29, 14}, {-127, 53}, {-266, 108}},
    {{-2, -38}, {-31, -7}, {-11, 2}, {20, 1}},
    {{-52, -112}, {-76, -15}, {-147, 82}, {-177, 137}},
    {{23, -28}, {-12, -4}, {0, 2}, {4, 7}},
    {{14, -15}, {-19, 2}, {-54, 15}, {-72, 17}},
    {{8, 4}, {-10, 11}, {-41, 19}, {-67, 8}},
    {{15, -35}, {-19, 4}, {-102, 82}, {-112, -24}},
    {{15, -11}, {-4, -1}, {-22, 5}, {-48, 15}},
    {{29, -19}, {-9, 9}, {-63, 43}, {-195, 115}},
    {{12, -66}, {-50, 30}, {-57, 78}, {-75, 154}},
    {{-39, -84}, {-51, -31}, {-81, 19}, {-123, 60}},
    {{24, -63}, {-63, 17}, {-107, 71}, {-2, 25}},
    {{29, -23}, {-4, 3}, {-105, 34}, {-98, -31}},
    {{-10, 8}, {-2, 17}, {-28, 30}, {8, -11}},
    {{21, -38}, {-43, -14}, {-173, 88}, {-8, 51}},
    {{-29, 12}, {43, -16}, {19, -10}, {-10, -34}},
    {{46, -9}, {-17, 5}, {-78, 18}, {-249, 91}},
    {{-19, -59}, {-67, 22}, {-69, 73}, {-17, 102}},
    {{-47, -83}, {-54, -34}, {-64, 7}, {-116, 58}},
    {{56, -57}, {-30, 4}, {-85, 52}, {-67, 72}},
    {{5, -16}, {-3, 3}, {-44, 27}, {-108, 54}},
    {{-10, 11}, {26, -6}, {67, -34}, {-2, -41}},
    {{14, -34}, {-77, 9}, {-210, 123}, {-74, -1}},
    {{-36, 10}, {18, -16}, {39, -36}, {-6, -28}},
    {{26, -13}, {-32, 13}, {-42, 24}, {-208, 100}},
    {{2, -84}, {1, 51}, {-16, 279}, {185, 279}},
    {{-48, -89}, {-78, -27}, {-173, 53}, {-269, 108}},
    {{58, -55}, {-92, 105}, {-53, 135}, {85, 157}},
    {{43, -29}, {-51, 56}, {-73, 101}, {-22, 89}},
    {{-5, -15}, {-7, -2}, {-21, -20}, {83, 29}},
    {{35, -35}, {-110, 47}, {-155, 59}, {-18, 58}},
    {{-13, 5}, {-14, 16}, {5, -16}, {10, -19}},
    {{85, -1}, {-74, 24}, {-160, 35}, {-312, 44}},
    {{-42, -64}, {-29, 60}, {52, 319}, {118, 198}},
    {{-44, -80}, {-63, -34}, {-90, 19}, {-262, 63}},
    {{52, -61}, {-109, 77}, {-112, 116}, {50, 115}},
    {{13, -11}, {28, 26}, {-62, 103}, {18, 92}},
    {{-8, -2}, {-3, -7}, {-22, -10}, {54, -32}},
    {{22, -51}, {-20, -9}, {-180, 80}, {-22, 49}},
    {{-20, 10}, {35, -18}, {1, -27}, {85, 43}},
    {{70, -12}, {-7, 6}, {-43, 1}, {-232, 92}},
};

#ifndef TUNE
constexpr
#endif
Score piece_evals[8][24] =
{
    {
        {59, 75}, {35, 73}, {28, -6}, {13, 25}, {12, 0}, {36, 35}, {6, 8}, {4, 22},
        {9, 55}, {11, 43}, {51, 56}, {7, 55}, {20, 5}, {8, 65}, {65, 13}, {63, -12},
        {-63, 13}, {-6, 46}, {-17, 67}, {15, 122}, {-51, -6}, {1, 30}, {-10, 74}, {20, 116},
    },
    {
        {57, 55}, {41, 53}, {25, -15}, {14, 21}, {13, 2}, {35, 25}, {4, 4}, {6, 16},
        {11, 39}, {13, 37}, {44, 32}, {12, 51}, {1, 16}, {39, 49}, {35, 25}, {71, -25},
        {9, 1}, {1, -25}, {42, 61}, {28, 17}, {14, -6}, {-19, -29}, {36, 84}, {20, 3},
    },
    {
        {42, 78}, {12, 89}, {52, -25}, {20, 22}, {9, 1}, {46, 16}, {1, 7}, {32, 4},
        {34, 24}, {26, 25}, {51, 24}, {6, 59}, {8, 21}, {66, 30}, {113, -20}, {45, -11},
        {3, 0}, {10, -25}, {35, 58}, {20, 54}, {0, 1}, {-1, -36}, {22, 88}, {3, 56},
    },
    {
        {45, 58}, {24, 71}, {54, -31}, {15, 22}, {19, -5}, {35, 17}, {-2, 4}, {29, 1},
        {5, 32}, {18, 27}, {57, 4}, {-2, 62}, {9, 23}, {57, 28}, {103, -17}, {36, -13},
        {20, -1}, {-63, -55}, {46, 56}, {-29, -20}, {26, 0}, {-105, -47}, {54, 77}, {-40, -21},
    },
    {
        {36, 63}, {39, 110}, {77, -28}, {17, 4}, {-17, 6}, {22, 11}, {1, 1}, {65, -16},
        {-2, 24}, {-27, 43}, {28, 27}, {1, 35}, {3, 37}, {25, 36}, {122, -25}, {1, 16},
        {5, -14}, {-24, -71}, {-10, 52}, {-2, 81}, {-2, 3}, {-16, -59}, {5, 82}, {0, 83},
    },
    {
        {5, 65}, {31, 115}, {89, -35}, {15, -1}, {-1, -1}, {45, 14}, {0, 5}, {51, -6},
        {-24, 44}, {-33, 48}, {20, 18}, {20, 29}, {8, 42}, {47, 21}, {99, -17}, {7, 6},
        {3, -9}, {-149, -61}, {39, 20}, {27, 19}, {28, -6}, {-155, -49}, {68, 44}, {24, 19},
    },
    {
        {38, 148}, {26, 125}, {84, -32}, {24, -8}, {-27, 13}, {-1, -7}, {-33, 7}, {14, -1},
        {-23, 33}, {-25, 44}, {19, 8}, {2, 26}, {-12, 55}, {13, 41}, {77, -15}, {0, 30},
        {-7, -30}, {2, -67}, {8, 70}, {36, 71}, {-9, 3}, {15, -56}, {11, 80}, {30, 71},
    },
    {
        {11, 76}, {34, 123}, {65, -38}, {3, 4}, {-9, 6}, {39, 10}, {-15, 2}, {16, 15},
        {-24, 53}, {-44, 68}, {15, 8}, {5, 20}, {11, 44}, {45, 50}, {67, -8}, {-1, 24},
        {1, -17}, {-40, -64}, {22, 37}, {3, 17}, {13, 2}, {-37, -19}, {25, 48}, {2, 21},
    },
};

template<Color C>
Score evaluate_pieces(const Mobility& mobility)
{
    Square king_sq = first_square(position.type_bb[KING] & position.color_bb[C]);
    alignas(32) int v[8] = {
        popcount(position.type_bb[PAWN] & position.color_bb[C] & king_attack[king_sq]),
        popcount(position.type_bb[PAWN] & position.color_bb[~C] & king_attack[king_sq]),
        popcount(position.type_bb[PAWN] & position.color_bb[C] & king_threat[king_sq] & ~king_attack[king_sq]),
        popcount(position.type_bb[PAWN] & position.color_bb[~C] & king_threat[king_sq] & ~king_attack[king_sq]),
        popcount(~position.type_bb[PAWN] & position.color_bb[C] & king_attack[king_sq]),
        popcount(~position.type_bb[PAWN] & position.color_bb[~C] & king_attack[king_sq]),
        popcount(~position.type_bb[PAWN] & position.color_bb[C] & king_threat[king_sq] & ~king_attack[king_sq]),
        popcount(~position.type_bb[PAWN] & position.color_bb[~C] & king_threat[king_sq] & ~king_attack[king_sq]),
    };

    int eval_offset = king_sq >> 4;
    if (C == BLACK)
        eval_offset ^= 3;
    eval_offset *= 2;
    eval_offset += ((king_sq & 7) >= 2 && (king_sq & 7) < 6);
    Score r{0, 0};
    for (int i = 0; i < 8; i++)
    {
        r += king_evals[8 * eval_offset + i][std::min(3, v[i])];
    }

    constexpr int FWD = C == WHITE ? 8 : -8;

    BitBoard own_pawns = position.type_bb[PAWN] & position.color_bb[C];
    BitBoard own_attack = mobility.attacks[C][PAWN];

    BitBoard opp_pawns = position.type_bb[PAWN] & position.color_bb[~C];
    BitBoard opp_attack = mobility.attacks[~C][PAWN];

    const Score* pe = piece_evals[eval_offset];
    r += pe[0] * popcount(own_attack & position.color_bb[~C] & (position.type_bb[KNIGHT] | position.type_bb[BISHOP]));
    r += pe[1] * popcount(own_attack & position.color_bb[~C] & (position.type_bb[ROOK] | position.type_bb[QUEEN]));

    if (position.type_bb[ROOK] && position.type_bb[PAWN])
    {
        BitBoard own_rooks = position.type_bb[ROOK] & position.color_bb[C];

        r += pe[2] * popcount(own_rooks & ~smear<-FWD>(position.type_bb[PAWN]));
        r += pe[3] * popcount(own_rooks & ~smear<-FWD>(own_pawns));
    }

    if (position.type_bb[KNIGHT])
    {
        BitBoard knights = position.type_bb[KNIGHT] & position.color_bb[C];
        BitBoard km = mobility.attacks[C][KNIGHT];

        r -= pe[4] * popcount(km & opp_attack & ~(position.color_bb[~C] & ~position.type_bb[PAWN]));
        r += pe[5] * popcount(km & position.color_bb[~C] & ~(position.type_bb[PAWN] | position.type_bb[KNIGHT]));
        r -= pe[6] * popcount(km & position.type_bb[PAWN] & (position.color_bb[C] | opp_attack));

        constexpr BitBoard OUTPOST_RANKS = static_cast<BitBoard>(C == WHITE ? 0x00ffffff00000000ULL : 0x00000000ffffff00ULL);
        r += pe[7] * popcount((knights | (km & ~position.color_bb[C])) & own_attack & ~opp_attack & OUTPOST_RANKS);
    }

    BitBoard bishops = position.type_bb[BISHOP] & position.color_bb[C];
    if (position.type_bb[BISHOP])
    {
        BitBoard bblock = shift_signed<FWD - 1>(bishops & KING_SIDE) | shift_signed<FWD + 1>(bishops & QUEEN_SIDE);
        r -= pe[8] * popcount(bblock & own_pawns);
        BitBoard bblock2 = shift_signed<2 * FWD - 2>(bishops & KING_SIDE) | shift_signed<2 * FWD + 2>(bishops & QUEEN_SIDE);
        r -= pe[9] * popcount(bblock2 & (own_pawns | (opp_pawns & opp_attack)));
        r += pe[10] * popcount(mobility.attacks[C][BISHOP] & position.color_bb[~C] & (position.type_bb[ROOK] | position.type_bb[QUEEN]));
    }

    r += pe[11] * popcount(mobility.attacks2[C] & position.color_bb[~C] & ~position.type_bb[PAWN]);

    BitBoard passed_pawns = own_pawns & ~smear<-FWD>(opp_pawns | opp_attack);
    if (passed_pawns)
    {
        r += pe[12] * popcount(passed_pawns & ~smear<-FWD>(shift_signed<-FWD>(position.all_bb())));

        constexpr BitBoard LAST_RANKS = static_cast<BitBoard>(C == WHITE ? 0x00ffff0000000000ULL : 0x0000000000ffff00ULL);
        r += pe[13] * popcount(passed_pawns & LAST_RANKS &
            ~smear<-FWD>(mobility.attacks[~C][KNIGHT] | mobility.attacks[~C][BISHOP] | mobility.attacks[~C][ROOK] | mobility.attacks[~C][QUEEN]));
    }

    BitBoard guarded = own_attack |
        mobility.attacks[C][KNIGHT] | mobility.attacks[C][BISHOP] | mobility.attacks[C][ROOK] | mobility.attacks[C][QUEEN];
    BitBoard safe_checks = ~guarded & (~king_attack[king_sq] | mobility.attacks2[~C]) & ~position.color_bb[~C] & (
        (knight_attack[king_sq] & mobility.attacks[~C][KNIGHT]) |
        (bishop_attack(king_sq, position.all_bb()) & (mobility.attacks[~C][BISHOP] | mobility.attacks[~C][QUEEN])) |
        (rook_attack(king_sq, position.all_bb()) & (mobility.attacks[~C][ROOK] | mobility.attacks[~C][QUEEN]))
    );
    r -= pe[14] * popcount(safe_checks);
    r -= pe[15] * popcount(king_attack[king_sq] & mobility.attacks2[~C] & ~guarded);
    r += pe[16 + !!(position.type_bb[QUEEN] & position.color_bb[C]) + 2 * !!((bishops & LIGHT_SQUARES) && (bishops & DARK_SQUARES)) + 4 * !!passed_pawns];

    return r;
}

alignas(64)
#ifndef TUNE
constexpr
#endif
Score king_relative_pawn_psqt[16][48] =
{
    {
        {13, -11}, {14, 4}, {5, 4}, {0, 5}, {-8, -10}, {-6, -2}, {-5, -5}, {2, -3},
        {15, -9}, {-1, -7}, {-1, -3}, {0, 8}, {1, -6}, {-2, -8}, {-4, -7}, {-4, 3},
        {20, -5}, {-3, -4}, {-2, -10}, {2, -2}, {-2, -5}, {-2, -14}, {1, -17}, {4, -10},
        {2, 13}, {1, -4}, {0, -5}, {14, -17}, {8, -16}, {6, -12}, {-5, -9}, {-3, -4},
        {11, 28}, {-11, 19}, {23, -21}, {52, -37}, {38, -27}, {3, -11}, {31, -10}, {-8, 9},
        {15, 40}, {7, 15}, {20, 14}, {29, -6}, {18, 4}, {22, -3}, {11, 24}, {-6, 21},
    },
    {
        {6, -11}, {10, 4}, {5, 4}, {-4, 2}, {3, -15}, {-7, -6}, {-2, -5}, {-1, 2},
        {8, -6}, {16, -6}, {-3, 5}, {-7, 4}, {-3, 2}, {-1, -2}, {-4, -2}, {-2, 1},
        {15, -7}, {11, -4}, {1, -4}, {-2, -3}, {-5, -3}, {-2, -8}, {2, -11}, {2, -4},
        {1, 5}, {4, 1}, {2, -4}, {4, -14}, {5, -13}, {5, -10}, {-1, -10}, {1, -3},
        {7, 17}, {-18, 28}, {11, -5}, {18, -23}, {18, -18}, {9, -11}, {9, -9}, {-5, 13},
        {-9, 8}, {-14, 3}, {10, 2}, {18, -14}, {10, -7}, {10, -2}, {7, -7}, {-13, 22},
    },
    {
        {1, -1}, {-2, -6}, {21, -1}, {2, -3}, {-4, -2}, {0, -5}, {-2, 1}, {0, 3},
        {-4, -4}, {3, -3}, {10, 0}, {5, -4}, {-4, -6}, {5, -2}, {-3, 4}, {-4, 1},
        {1, -1}, {2, -1}, {11, 1}, {5, 1}, {0, -7}, {2, -3}, {-3, -1}, {-7, -2},
        {0, -2}, {9, 0}, {18, -5}, {12, -10}, {8, -13}, {7, -10}, {-2, -10}, {-7, -2},
        {-3, 1}, {-5, 2}, {-14, 0}, {13, -19}, {8, -11}, {-1, -6}, {1, -16}, {1, 0},
        {14, -3}, {-17, -2}, {-12, -2}, {-3, -9}, {22, -12}, {1, -19}, {-3, -2}, {11, -17},
    },
    {
        {7, -1}, {-1, -6}, {-2, -3}, {12, 6}, {2, 3}, {-1, -7}, {6, 2}, {0, 3},
        {-5, 5}, {-8, -1}, {5, 0}, {17, -1}, {1, -1}, {-4, -1}, {-1, 11}, {-2, 0},
        {-6, 1}, {-1, 0}, {8, 4}, {9, 5}, {10, -5}, {0, 1}, {0, -1}, {-7, 5},
        {-2, -3}, {1, -4}, {6, -2}, {5, -1}, {3, -1}, {5, -8}, {1, -9}, {-7, -4},
        {-4, 0}, {-21, 9}, {-22, 6}, {-19, 5}, {-2, -11}, {-14, -2}, {-8, -12}, {-8, 6},
        {-2, -15}, {-13, -2}, {-16, -17}, {-13, 8}, {9, -11}, {-17, -11}, {-21, -15}, {9, -5},
    },
    {
        {-10, -12}, {-7, 2}, {2, -1}, {-17, -1}, {1, 3}, {-10, 2}, {3, -10}, {3, -1},
        {11, -5}, {17, 3}, {5, 2}, {12, 8}, {19, 0}, {2, -13}, {1, -5}, {-3, -1},
        {12, -15}, {8, -3}, {-8, 9}, {9, 10}, {7, 6}, {-10, -6}, {2, -14}, {5, -9},
        {6, -3}, {-6, 9}, {-2, 16}, {20, -3}, {14, -6}, {-1, -16}, {4, -12}, {1, -12},
        {19, 35}, {8, 19}, {11, 5}, {18, -19}, {10, -24}, {6, -25}, {17, -40}, {2, -25},
        {27, 37}, {-3, 32}, {19, 6}, {-3, -16}, {5, -25}, {-2, -36}, {7, -19}, {12, -23},
    },
    {
        {-6, -10}, {-7, -11}, {-3, -1}, {-13, -8}, {-2, 3}, {-8, 1}, {-3, 2}, {-3, 3},
        {5, -12}, {3, 5}, {11, 0}, {-1, 2}, {-5, 7}, {5, -2}, {1, 0}, {1, 1},
        {14, -4}, {6, -6}, {-6, 13}, {-9, 8}, {1, 10}, {-6, 8}, {3, -6}, {3, -2},
        {3, -3}, {8, 7}, {-2, 13}, {8, -2}, {9, 0}, {8, -13}, {3, -8}, {0, -5},
        {20, 16}, {31, 15}, {30, 14}, {25, -14}, {5, -22}, {13, -33}, {18, -35}, {13, -19},
        {23, -4}, {2, 40}, {34, 20}, {6, -12}, {8, -32}, {0, -42}, {5, -27}, {13, -18},
    },
    {
        {-6, -8}, {-14, -5}, {5, -14}, {0, -14}, {-1, -13}, {-2, -2}, {0, 3}, {-6, 13},
        {-7, 0}, {-8, -6}, {6, 2}, {13, -4}, {2, 1}, {6, 1}, {9, 0}, {-1, 3},
        {-3, 1}, {-2, -4}, {11, -1}, {4, 9}, {3, 7}, {0, 9}, {1, 0}, {2, 5},
        {-3, -5}, {14, -3}, {16, 7}, {2, 10}, {-1, 0}, {1, 0}, {-1, -2}, {1, -4},
        {19, -3}, {8, 10}, {14, 15}, {1, 6}, {12, -20}, {0, -18}, {3, -25}, {5, -6},
        {28, -18}, {-6, 3}, {4, -2}, {3, -9}, {1, -24}, {-24, -40}, {-5, -35}, {19, -28},
    },
    {
        {-7, 13}, {-12, -4}, {2, -14}, {-2, -20}, {-3, -16}, {1, -11}, {-1, 2}, {-8, 16},
        {-1, 5}, {-9, 0}, {8, -9}, {-1, 0}, {4, -3}, {4, -6}, {-1, 1}, {2, 8},
        {4, 5}, {-1, 0}, {-1, -1}, {9, 2}, {3, 3}, {-4, 3}, {1, 6}, {0, 7},
        {0, -2}, {3, -4}, {18, -2}, {7, 12}, {2, 4}, {0, -4}, {0, 3}, {7, -4},
        {13, -4}, {-3, -12}, {7, 10}, {14, 7}, {9, 1}, {1, -13}, {-2, -22}, {8, -13},
        {22, -29}, {-10, -16}, {10, -10}, {6, -3}, {9, -13}, {-4, -27}, {-6, -49}, {14, -24},
    },
    {
        {1, 26}, {-23, 22}, {-11, 0}, {-3, 2}, {-8, -24}, {-19, 5}, {-14, 8}, {1, 6},
        {7, 8}, {-4, 3}, {-2, -7}, {-21, -3}, {-4, 10}, {-5, -9}, {-11, 12}, {-4, -3},
        {1, -5}, {1, -8}, {-5, -2}, {2, 24}, {5, 19}, {-7, 5}, {1, -7}, {13, -11},
        {1, -10}, {-3, 16}, {21, 1}, {1, 21}, {25, 6}, {1, -8}, {-3, 3}, {-7, -1},
        {3, -6}, {8, 19}, {14, 11}, {27, -8}, {2, -14}, {-1, -29}, {10, -42}, {1, -32},
        {10, 23}, {4, 21}, {11, 5}, {-1, -6}, {-2, -17}, {-12, -26}, {-6, -24}, {5, -68},
    },
    {
        {-4, 22}, {-16, 17}, {4, 10}, {1, -9}, {-5, -8}, {-23, 3}, {0, 3}, {5, 6},
        {-14, -6}, {-3, 3}, {-6, -8}, {-21, -17}, {8, 23}, {1, 6}, {-9, 9}, {6, 1},
        {-5, -6}, {-6, -4}, {-5, -1}, {-11, -8}, {5, 25}, {1, 5}, {6, -1}, {13, -8},
        {12, 1}, {-6, 31}, {20, 14}, {2, -6}, {21, 1}, {3, 1}, {-2, -9}, {-5, -5},
        {29, 1}, {1, 15}, {14, 12}, {27, -8}, {5, 0}, {0, -25}, {7, -43}, {-3, -42},
        {13, -6}, {6, 9}, {10, 10}, {-1, -12}, {1, -10}, {-15, -38}, {-10, -45}, {2, -70},
    },
    {
        {-2, 11}, {-19, 11}, {-2, 11}, {5, 6}, {-7, -10}, {-4, 1}, {-4, 14}, {-4, 8},
        {-29, 7}, {-10, 4}, {3, 5}, {-3, -4}, {-15, -4}, {-2, 0}, {-7, 11}, {-3, 7},
        {-9, -3}, {-17, -19}, {1, 0}, {2, -9}, {-2, -3}, {12, 10}, {-3, 4}, {2, 4},
        {-5, -3}, {-6, -6}, {3, 33}, {-9, 12}, {6, 4}, {5, 19}, {10, -3}, {3, -8},
        {36, 6}, {1, 6}, {-4, 10}, {-7, 24}, {5, 7}, {-1, -14}, {-4, -26}, {-2, -28},
        {12, -24}, {7, -8}, {12, 19}, {5, 11}, {-3, 2}, {-16, -30}, {-22, -54}, {-6, -63},
    },
    {
        {-6, 6}, {-13, 9}, {-11, 6}, {0, 6}, {-5, -6}, {-6, 1}, {-5, 14}, {0, 14},
        {-20, 11}, {-13, -8}, {-2, -8}, {4, 2}, {-12, -6}, {-11, -10}, {-12, 7}, {-1, 13},
        {-6, 22}, {-14, -10}, {-3, -11}, {1, 3}, {-4, -11}, {6, -10}, {2, 8}, {1, 7},
        {-5, 10}, {-1, -6}, {-2, 3}, {5, 38}, {0, -5}, {7, 3}, {8, 6}, {1, 5},
        {32, -4}, {2, -1}, {-1, 6}, {3, 27}, {-2, 6}, {2, 11}, {-3, -14}, {8, -19},
        {7, -25}, {-1, -28}, {7, 2}, {8, 13}, {5, 11}, {-15, -16}, {-19, -32}, {-5, -59},
    },
    {
        {0, 29}, {6, 1}, {-10, -7}, {-2, 4}, {-7, -18}, {-13, 1}, {-3, 3}, {-2, -1},
        {0, 17}, {8, 14}, {1, 8}, {-21, -12}, {-8, 2}, {-1, -2}, {-12, -8}, {-7, 0},
        {11, 43}, {4, 32}, {0, 20}, {4, 11}, {-1, 17}, {-5, -4}, {-2, -18}, {0, 0},
        {2, 7}, {-5, 11}, {12, -7}, {9, 7}, {18, -9}, {0, -7}, {-6, -10}, {-8, -6},
        {4, -32}, {0, 9}, {11, 13}, {24, -7}, {-3, -22}, {-1, -21}, {7, -32}, {-5, -31},
        {-9, -55}, {0, 18}, {7, 5}, {-7, -5}, {-4, -9}, {-13, -20}, {-2, -10}, {-1, -64},
    },
    {
        {-2, 15}, {3, -4}, {-9, -10}, {-4, 4}, {-4, -11}, {-7, 7}, {-9, -5}, {0, 7},
        {0, 11}, {5, 16}, {2, 3}, {-17, -1}, {-8, 0}, {-4, 5}, {-6, -2}, {-8, 2},
        {5, 38}, {3, 48}, {-2, 21}, {8, 15}, {-4, 7}, {-3, 6}, {-2, -4}, {-5, -1},
        {-4, -6}, {-8, 13}, {11, 10}, {-2, -16}, {25, -3}, {2, -7}, {-3, -3}, {-6, -7},
        {9, 3}, {-4, 8}, {14, 4}, {24, -5}, {4, -13}, {2, -18}, {-3, -45}, {0, -31},
        {2, -8}, {-7, 1}, {21, 22}, {-10, -19}, {-5, -11}, {-14, -23}, {-3, -16}, {-2, -68},
    },
    {
        {6, 4}, {-7, 3}, {-4, 6}, {9, 3}, {-8, -15}, {-4, -5}, {-3, 10}, {0, 4},
        {-16, -5}, {-7, 2}, {5, 14}, {8, -2}, {-24, -5}, {-13, 0}, {-4, 1}, {-6, 3},
        {-13, 40}, {-5, 22}, {16, 48}, {-2, 41}, {4, 11}, {4, 14}, {-1, -3}, {-4, -5},
        {-5, -27}, {10, 3}, {13, 32}, {4, 40}, {-15, -21}, {5, 8}, {8, 1}, {-5, -2},
        {23, -21}, {-9, -10}, {11, 27}, {-5, 11}, {-8, 4}, {-5, -20}, {-8, -25}, {-10, -42},
        {4, -44}, {-4, -29}, {7, 12}, {4, 24}, {-2, -6}, {-22, -28}, {-10, -32}, {-3, -46},
    },
    {
        {-3, -7}, {-3, 4}, {-2, 4}, {6, 7}, {-6, -8}, {-9, -18}, {1, 15}, {-2, 1},
        {-11, 9}, {-8, -1}, {2, 14}, {5, 7}, {-18, -5}, {-14, 3}, {-4, 7}, {-1, 12},
        {-5, 24}, {-8, 14}, {15, 35}, {9, 42}, {6, 15}, {10, 24}, {1, 2}, {-2, 1},
        {9, 7}, {-4, -37}, {9, 12}, {-1, 31}, {3, 11}, {-7, -17}, {11, 7}, {-1, 0},
        {23, -12}, {-10, -17}, {2, 2}, {1, 42}, {-4, 4}, {-1, -14}, {-9, -19}, {-7, -26},
        {-1, -39}, {-4, -44}, {-2, 2}, {4, 8}, {7, 14}, {-22, -44}, {-8, -26}, {-7, -50},
    },
};

alignas(64)
#ifndef TUNE
constexpr
#endif
Score king_relative_knight_psqt[16][64] =
{
    {
        {6, -14}, {0, -9}, {2, -6}, {8, -7}, {0, -7}, {2, -12}, {-4, -39}, {-15, -20},
        {15, 9}, {1, 4}, {-10, -7}, {2, -5}, {-1, -8}, {-1, -5}, {-2, -27}, {-2, -23},
        {-1, 15}, {1, -4}, {0, 1}, {3, 7}, {0, 0}, {2, -8}, {-1, -6}, {-17, -14},
        {2, 4}, {2, 11}, {-2, 9}, {3, 12}, {4, -2}, {2, 5}, {3, -2}, {-9, -14},
        {3, 9}, {11, 11}, {15, 6}, {8, 3}, {17, -1}, {4, 3}, {-6, -4}, {1, -16},
        {7, -3}, {-3, 11}, {15, -8}, {23, -11}, {10, 8}, {6, 5}, {7, 3}, {-3, -7},
        {-6, -2}, {-5, -1}, {10, 1}, {6, 2}, {7, 0}, {-5, -6}, {0, 0}, {-14, 2},
        {-12, -7}, {-10, -5}, {19, 5}, {2, 6}, {12, -5}, {1, 0}, {7, 8}, {34, 5},
    },
    {
        {2, -13}, {-2, -6}, {3, -3}, {0, -11}, {1, -9}, {0, -6}, {5, -6}, {-12, -21},
        {5, 5}, {-1, 3}, {-6, 0}, {-1, 3}, {5, -6}, {-1, -5}, {1, -11}, {-2, -22},
        {-2, 0}, {7, 4}, {0, 0}, {-5, 7}, {-5, 6}, {3, -1}, {4, -2}, {1, -13},
        {-2, 0}, {1, 6}, {-2, 9}, {-1, 10}, {1, 3}, {3, 0}, {3, 8}, {8, -8},
        {3, 7}, {0, 8}, {0, 9}, {4, 0}, {3, 5}, {3, 4}, {-3, 9}, {6, -2},
        {7, 5}, {-4, 9}, {18, 2}, {15, -5}, {7, -3}, {-1, 6}, {8, 1}, {11, -7},
        {-7, -3}, {-6, 1}, {7, 1}, {7, 3}, {4, -1}, {3, -7}, {11, 2}, {11, 10},
        {-12, -8}, {-10, -6}, {17, -3}, {10, 9}, {27, -3}, {9, 0}, {5, 7}, {33, 13},
    },
    {
        {7, -13}, {-10, 18}, {1, 19}, {-2, 2}, {2, 11}, {-5, -8}, {-10, 4}, {12, -44},
        {3, -5}, {5, 10}, {-2, 21}, {1, 5}, {-2, -4}, {-2, -5}, {-3, -8}, {-5, -21},
        {-2, -2}, {-4, 12}, {7, 12}, {0, 4}, {-6, 5}, {-3, 4}, {0, 0}, {2, -21},
        {-1, 3}, {0, 4}, {1, 2}, {-1, 4}, {-1, 3}, {-4, -2}, {-8, 9}, {-5, -3},
        {-12, 6}, {1, 2}, {-2, -3}, {5, -2}, {3, 0}, {-3, 4}, {-4, -2}, {-9, 2},
        {-7, -10}, {4, 5}, {-15, 11}, {-6, 6}, {-8, 2}, {-21, 15}, {-1, 8}, {-5, 8},
        {-4, -3}, {-11, 3}, {-12, 2}, {-12, 10}, {-1, -2}, {-6, 2}, {13, -8}, {4, 1},
        {-10, -5}, {-13, -7}, {12, 1}, {-5, 5}, {7, -4}, {-10, -1}, {-2, 0}, {1, -16},
    },
    {
        {13, -15}, {4, 1}, {0, 17}, {-1, 17}, {2, 9}, {4, -7}, {2, 3}, {16, -29},
        {-6, -20}, {4, 10}, {-2, 22}, {6, 4}, {-1, 0}, {2, -4}, {0, -3}, {-6, -20},
        {-1, 1}, {-3, 7}, {-1, 7}, {6, 7}, {-3, 10}, {-1, 5}, {-3, 10}, {3, -7},
        {1, -3}, {-5, 1}, {2, 1}, {-10, 15}, {-3, 6}, {3, 3}, {-14, 14}, {-2, 0},
        {-6, 7}, {-5, 3}, {1, 0}, {-4, -5}, {-3, 7}, {-9, 8}, {-5, 9}, {-5, 8},
        {14, -8}, {-5, 5}, {-22, 14}, {-14, 0}, {-7, 8}, {-20, 17}, {-2, 2}, {-8, -1},
        {-5, 7}, {-13, 3}, {-9, 8}, {-33, 8}, {-3, -3}, {-6, 4}, {17, -6}, {7, -2},
        {-20, -8}, {-16, -10}, {6, -9}, {-5, 4}, {8, -4}, {-15, -3}, {2, 5}, {8, -15},
    },
    {
        {7, 12}, {3, 21}, {-5, 20}, {-9, 7}, {-4, -1}, {-13, -8}, {-49, 3}, {-30, -8},
        {3, 13}, {-9, 19}, {-7, 13}, {-3, 5}, {-8, -6}, {-17, 7}, {-9, -26}, {-9, -4},
        {-6, 32}, {16, 15}, {2, 0}, {1, 2}, {7, -3}, {-14, -14}, {-25, -6}, {-23, 5},
        {3, 11}, {4, -9}, {14, 1}, {21, -1}, {12, -2}, {16, -12}, {1, -21}, {-25, 0},
        {-4, 11}, {4, -1}, {9, -1}, {22, -4}, {18, -14}, {17, -20}, {-8, -7}, {-30, -8},
        {1, 12}, {7, 4}, {15, -2}, {11, -2}, {6, -3}, {9, -12}, {5, 11}, {-2, -11},
        {0, 14}, {10, 11}, {0, 7}, {12, -2}, {9, 2}, {-8, -4}, {-5, -5}, {-12, -8},
        {-12, 8}, {7, 7}, {19, 15}, {3, 3}, {7, -5}, {2, 4}, {-10, -3}, {11, 11},
    },
    {
        {6, 13}, {2, 19}, {-5, 22}, {-1, 4}, {-2, -2}, {-12, -3}, {-56, 4}, {-26, -4},
        {7, 16}, {-9, 16}, {-6, 15}, {-4, 3}, {-4, -5}, {-17, 10}, {-6, -23}, {-7, -5},
        {-4, 29}, {14, 14}, {3, 2}, {-2, -2}, {1, 0}, {-1, -6}, {-7, -3}, {-29, 3},
        {1, 12}, {5, 1}, {-22, 1}, {4, -2}, {11, 2}, {14, -15}, {0, -10}, {-17, 2},
        {-2, 11}, {14, 2}, {17, -7}, {17, -10}, {18, -8}, {17, -15}, {-5, -8}, {-25, -3},
        {-2, 8}, {7, 0}, {19, -10}, {22, 0}, {1, 0}, {13, -8}, {1, 9}, {-3, -10},
        {1, 16}, {9, 11}, {3, 11}, {17, 3}, {9, -2}, {-8, -1}, {-2, -1}, {-8, -5},
        {-17, 7}, {6, 6}, {18, 13}, {4, 2}, {7, -2}, {-1, -2}, {-11, -3}, {8, 9},
    },
    {
        {4, 11}, {-8, 18}, {-5, 18}, {-8, 5}, {-5, 12}, {-4, 11}, {-42, 16}, {-27, -6},
        {3, 16}, {-11, 29}, {-5, 27}, {2, 8}, {2, -2}, {-14, 13}, {3, 9}, {-10, -3},
        {-6, 17}, {2, 13}, {-1, 4}, {9, 2}, {0, -6}, {-3, -7}, {-6, -4}, {-17, 1},
        {-5, 17}, {-3, 6}, {-2, -3}, {-1, -7}, {10, -15}, {28, -11}, {-3, -6}, {-7, 4},
        {-8, 9}, {1, -7}, {43, -13}, {15, -11}, {11, -19}, {24, -14}, {0, -3}, {-15, 9},
        {-3, 10}, {2, 7}, {3, 1}, {9, -1}, {12, -3}, {-4, -2}, {0, -4}, {-1, -3},
        {-7, 2}, {15, 7}, {1, 5}, {2, -3}, {4, -4}, {2, -4}, {-3, -1}, {-1, -1},
        {-12, 0}, {8, 16}, {23, 10}, {3, 6}, {3, -2}, {1, 2}, {-1, 3}, {4, 4},
    },
    {
        {4, 8}, {3, 30}, {-5, 19}, {-7, 6}, {-5, 19}, {-2, 11}, {-24, 27}, {-30, -4},
        {0, 15}, {-14, 28}, {-6, 26}, {5, 15}, {8, 1}, {-13, 12}, {-7, 12}, {-7, -5},
        {-6, 10}, {-8, -1}, {-1, 2}, {3, 2}, {6, -4}, {2, 0}, {-4, 2}, {-19, 1},
        {-6, 15}, {-4, 6}, {4, -4}, {0, -6}, {7, -11}, {17, -23}, {-1, -7}, {-9, 3},
        {-12, 7}, {-2, -6}, {40, -29}, {14, -9}, {10, -15}, {23, -19}, {-4, -6}, {-27, 0},
        {-4, 0}, {-3, 6}, {4, 2}, {10, 2}, {11, -6}, {-4, -4}, {-3, 0}, {-2, -1},
        {-3, 3}, {1, 10}, {-1, 2}, {4, -1}, {5, -2}, {7, -3}, {-3, 0}, {-1, -4},
        {-15, 1}, {7, 13}, {21, 10}, {5, 5}, {3, -4}, {-1, 2}, {-4, 0}, {3, 1},
    },
    {
        {1, 6}, {-6, 0}, {-5, -3}, {-10, -6}, {-10, -8}, {-5, 3}, {2, 14}, {4, 11},
        {-5, -2}, {3, -4}, {-4, 4}, {3, -3}, {-13, -6}, {-4, -44}, {-9, -17}, {-15, -6},
        {-1, 2}, {-8, -4}, {3, -21}, {-13, -12}, {-4, -21}, {1, -15}, {-11, -27}, {-5, -13},
        {1, 6}, {-6, -4}, {3, -8}, {1, -18}, {6, -22}, {9, -28}, {-8, -29}, {-1, -8},
        {18, 13}, {0, -23}, {1, -19}, {6, -16}, {1, -12}, {3, -20}, {-2, -21}, {-5, -15},
        {-2, 1}, {12, 21}, {1, -3}, {1, 6}, {5, -10}, {-6, -6}, {-5, -11}, {-19, -11},
        {2, 11}, {0, 16}, {0, -1}, {2, 2}, {0, -9}, {-6, -8}, {-7, -6}, {-6, -8},
        {3, 4}, {6, 1}, {1, -9}, {0, 2}, {-15, -24}, {4, -14}, {1, -2}, {-1, -9},
    },
    {
        {1, 2}, {-4, 1}, {-9, -4}, {-4, -1}, {-8, -4}, {-7, -4}, {0, 22}, {1, 10},
        {0, 6}, {4, -1}, {-6, 4}, {-1, -1}, {-11, 0}, {-4, -44}, {-10, -14}, {-16, -7},
        {-2, 2}, {-10, -7}, {6, -9}, {-13, -7}, {0, -15}, {-2, -17}, {-7, -23}, {-8, -15},
        {2, 11}, {-1, 4}, {2, -6}, {0, -19}, {10, -20}, {9, -32}, {-6, -28}, {4, -5},
        {15, 16}, {-1, -23}, {1, -15}, {12, -12}, {-2, -15}, {2, -21}, {-2, -20}, {-4, -15},
        {1, 3}, {10, 21}, {2, -2}, {0, -4}, {4, -11}, {-6, -6}, {-3, -10}, {-17, -12},
        {3, 15}, {2, 14}, {2, -3}, {-1, 1}, {0, -11}, {-6, -11}, {-6, -7}, {-11, -15},
        {1, 4}, {4, 1}, {7, -5}, {0, 1}, {-13, -24}, {10, -12}, {-1, -5}, {-3, -4},
    },
    {
        {0, 3}, {0, 5}, {-8, -3}, {-3, -7}, {-12, -3}, {-3, -4}, {-7, 14}, {-2, 7},
        {-10, -7}, {4, 2}, {-14, -5}, {-2, 4}, {-15, -1}, {-3, -28}, {-9, -19}, {-18, -11},
        {-8, -6}, {-13, 1}, {3, -13}, {-12, -10}, {-3, -16}, {1, -17}, {-8, -26}, {-9, -13},
        {5, 14}, {-2, -1}, {5, -12}, {15, -15}, {13, -27}, {8, -28}, {-7, -31}, {6, -1},
        {12, 7}, {14, -8}, {0, -14}, {10, -9}, {2, -14}, {11, -21}, {-3, -22}, {-9, -17},
        {1, 0}, {14, 19}, {2, 3}, {5, 2}, {8, -12}, {-11, -10}, {-6, -8}, {-21, -12},
        {-1, 7}, {6, 15}, {-2, 7}, {1, 9}, {2, -1}, {-8, -13}, {-3, -5}, {-8, -15},
        {0, 2}, {5, -1}, {3, -9}, {3, 1}, {-12, -30}, {8, -13}, {-2, -10}, {0, -5},
    },
    {
        {1, 6}, {-1, 6}, {-6, -7}, {-8, -12}, {-9, 0}, {0, 2}, {-6, 14}, {0, 11},
        {-9, -6}, {5, 5}, {-9, 6}, {-3, -6}, {-14, 2}, {-2, -28}, {-7, -17}, {-18, -8},
        {-7, -9}, {-11, 1}, {5, -13}, {-9, -11}, {-2, -18}, {-1, -17}, {-7, -23}, {-8, -18},
        {4, 10}, {-3, -10}, {5, -9}, {5, -19}, {11, -23}, {10, -24}, {-5, -27}, {5, -1},
        {13, 10}, {12, -16}, {3, -12}, {9, -17}, {5, -13}, {12, -19}, {-4, -23}, {-7, -19},
        {-1, -3}, {13, 18}, {1, 0}, {8, 5}, {10, -7}, {-10, -6}, {-6, -6}, {-20, -12},
        {2, 11}, {6, 16}, {0, 2}, {5, 8}, {0, -5}, {-5, -8}, {-2, -2}, {-7, -15},
        {2, 3}, {2, -1}, {5, -9}, {3, 1}, {-11, -30}, {6, -9}, {-3, -10}, {0, -8},
    },
    {
        {-4, 8}, {-2, 3}, {-4, 0}, {-4, -4}, {-9, -17}, {-2, -1}, {0, 10}, {-4, -4},
        {-5, -5}, {0, -1}, {-7, -7}, {-8, -1}, {2, -2}, {-6, -29}, {-12, -13}, {-6, -5},
        {1, 4}, {-6, -13}, {-4, -19}, {-6, -19}, {-9, -13}, {-8, -28}, {-4, -18}, {-6, -5},
        {-3, 2}, {-4, -8}, {1, -16}, {2, -20}, {7, -21}, {4, -29}, {-1, -15}, {-4, -1},
        {-4, -7}, {2, -18}, {-7, -14}, {0, -15}, {-4, -10}, {-8, -20}, {-5, -15}, {-2, -6},
        {4, -3}, {-4, 5}, {1, -7}, {1, -13}, {-2, -9}, {-2, -17}, {-10, -14}, {-10, -8},
        {-5, 0}, {-1, 9}, {2, 7}, {-1, 3}, {-3, -16}, {-6, -5}, {-4, -4}, {-3, -7},
        {-2, 1}, {1, -3}, {-5, -7}, {3, 5}, {-10, -24}, {-1, -6}, {-2, -7}, {-5, -8},
    },
    {
        {-6, 5}, {-1, 0}, {-8, -3}, {-8, -6}, {-9, -18}, {-1, -3}, {5, 12}, {-3, -1},
        {-6, -8}, {-1, -5}, {-6, -7}, {-12, -7}, {-1, -4}, {-3, -29}, {-10, -11}, {-6, -3},
        {-1, 2}, {-6, -12}, {-2, -14}, {-2, -15}, {-5, -15}, {-8, -28}, {-8, -24}, {-9, -7},
        {-1, 2}, {-5, -8}, {4, -13}, {3, -19}, {8, -19}, {7, -24}, {-2, -13}, {-8, -6},
        {-4, -9}, {3, -19}, {-6, -7}, {2, -12}, {0, -9}, {-8, -23}, {-7, -17}, {-7, -5},
        {8, -3}, {-2, 8}, {-2, -3}, {2, -9}, {-2, -10}, {0, -14}, {-10, -15}, {-13, -8},
        {-6, 3}, {0, 8}, {-1, 6}, {2, 6}, {-2, -14}, {-10, -6}, {-4, -4}, {-6, -10},
        {-2, -1}, {2, 0}, {-5, -10}, {2, 6}, {-11, -26}, {-3, -7}, {-2, -8}, {-7, -12},
    },
    {
        {-5, 10}, {-2, -1}, {-6, -3}, {-3, 1}, {-8, -16}, {-5, -8}, {2, 9}, {-3, 0},
        {-8, -7}, {-1, 0}, {-3, -5}, {-6, -5}, {0, -9}, {-5, -32}, {-8, -15}, {-2, -5},
        {-2, 7}, {-6, -16}, {-12, -29}, {-9, -25}, {-5, -11}, {-7, -31}, {-10, -21}, {-8, -8},
        {-2, 2}, {-6, -12}, {-4, -20}, {4, -16}, {-1, -29}, {1, -32}, {-5, -21}, {-7, -6},
        {0, -3}, {-6, -24}, {-5, -10}, {1, -17}, {-5, -16}, {-7, -24}, {-8, -17}, {-9, -8},
        {1, -8}, {4, 11}, {-3, -6}, {-4, -16}, {-4, -11}, {-3, -24}, {-13, -14}, {-8, -4},
        {-8, -1}, {-3, 15}, {3, -3}, {-7, -3}, {-7, -19}, {-6, -8}, {-10, -4}, {-14, -15},
        {-5, -1}, {-4, -7}, {-8, -7}, {1, 2}, {-10, -23}, {-2, -7}, {1, -6}, {-10, -8},
    },
    {
        {-2, 15}, {-2, -4}, {-6, -2}, {-4, -1}, {-8, -16}, {-2, -11}, {0, 11}, {-6, 0},
        {-5, -9}, {-2, -2}, {-3, -5}, {-9, -7}, {2, -5}, {-6, -34}, {-7, -10}, {-3, 2},
        {0, 8}, {-9, -19}, {-12, -25}, {-5, -20}, {0, -9}, {-7, -33}, {-8, -21}, {-9, -6},
        {-4, 0}, {-5, -12}, {-1, -22}, {2, -16}, {3, -27}, {1, -33}, {-5, -16}, {-2, -1},
        {-2, -6}, {-2, -21}, {-7, -11}, {0, -13}, {-6, -11}, {-8, -24}, {-8, -21}, {-9, -8},
        {-1, -9}, {-2, 10}, {0, -3}, {0, -15}, {-4, -10}, {-3, -25}, {-13, -13}, {-7, -6},
        {-6, 0}, {1, 16}, {3, -7}, {-1, 2}, {-4, -15}, {-9, -9}, {-7, -3}, {-10, -9},
        {-5, -2}, {-4, -8}, {-6, -10}, {3, 0}, {-8, -18}, {-6, -9}, {1, -9}, {-8, -4},
    },
};

alignas(64)
#ifndef TUNE
constexpr
#endif
Score king_relative_opp_pawn_psqt[8][48] =
{
    {
        {12, 36}, {5, 35}, {-5, 20}, {-8, -16}, {-12, -18}, {-2, -38}, {3, -39}, {21, -33},
        {-6, 20}, {-19, 2}, {-19, 14}, {-5, 1}, {7, -25}, {9, -33}, {10, -20}, {10, -25},
        {-3, 11}, {8, 3}, {-5, 5}, {-2, 0}, {-2, 2}, {2, -10}, {0, -8}, {1, -10},
        {2, 6}, {3, 4}, {-1, 1}, {0, 0}, {-2, 2}, {-2, -1}, {-2, -5}, {-2, -5},
        {2, 1}, {3, 0}, {0, -4}, {-1, -3}, {-2, 1}, {-3, 0}, {-5, -4}, {-4, -2},
        {1, 6}, {1, 1}, {1, -4}, {-2, -2}, {-3, -4}, {-4, 1}, {-4, -1}, {-2, -1},
    },
    {
        {-4, 25}, {4, 19}, {-2, 36}, {-5, 28}, {-4, 2}, {8, 1}, {2, -35}, {18, -53},
        {-12, 22}, {-16, 32}, {-21, 15}, {-11, 6}, {-11, 23}, {4, -2}, {5, -11}, {5, -23},
        {-6, 5}, {-10, 3}, {-7, 5}, {1, -5}, {-1, 0}, {0, 1}, {-4, -2}, {0, -8},
        {-4, 4}, {-6, 3}, {0, 2}, {1, -1}, {-3, 0}, {-1, 2}, {-3, -2}, {-2, -4},
        {-1, 1}, {-3, 2}, {1, 1}, {1, 1}, {-2, 1}, {-2, 0}, {-4, 2}, {-2, 0},
        {-1, -3}, {-1, 1}, {3, 0}, {1, 6}, {0, 0}, {-3, 2}, {-2, 1}, {-2, -2},
    },
    {
        {9, 45}, {10, 41}, {0, 63}, {-7, 10}, {-14, 10}, {-10, -1}, {-2, -5}, {-2, -1},
        {-19, 57}, {-20, 49}, {-13, 42}, {-1, 24}, {-9, -8}, {-1, -20}, {-15, -13}, {-7, -3},
        {-5, 5}, {-8, 0}, {1, 7}, {0, 14}, {1, -1}, {10, -8}, {0, -10}, {6, -8},
        {-1, -3}, {-6, -10}, {3, 1}, {1, 1}, {4, 2}, {-3, -1}, {6, -4}, {0, -1},
        {-5, -1}, {0, -4}, {-6, 2}, {-1, -2}, {3, 3}, {2, 3}, {3, 2}, {-2, 5},
        {-17, 3}, {-3, -1}, {0, -3}, {1, -1}, {-4, 5}, {-4, 8}, {1, 5}, {0, 5},
    },
    {
        {-2, 23}, {7, 39}, {-1, 62}, {-2, 46}, {-2, 30}, {-7, 12}, {-4, -5}, {-2, 0},
        {-20, 39}, {-18, 53}, {-15, 31}, {0, 27}, {-7, 28}, {2, 19}, {-4, 5}, {-9, -9},
        {-10, 17}, {-3, 9}, {-2, 0}, {-3, -4}, {9, 4}, {11, 4}, {4, -1}, {6, -8},
        {4, -2}, {-6, -1}, {0, -10}, {3, -14}, {0, -5}, {-1, -2}, {4, -2}, {2, -6},
        {0, -2}, {-2, -2}, {-1, -3}, {0, -3}, {-2, -1}, {1, 1}, {1, 1}, {-1, 0},
        {-15, 1}, {1, 1}, {1, 0}, {-2, -1}, {-3, 2}, {-2, 6}, {1, 2}, {-1, -2},
    },
    {
        {1, -2}, {-12, -1}, {-2, -6}, {-4, -9}, {-5, -11}, {-14, -8}, {-11, -4}, {-12, 10},
        {-1, 12}, {4, 17}, {-1, -3}, {-2, -30}, {-3, -32}, {-16, -32}, {-12, -18}, {-14, -18},
        {5, 9}, {2, 19}, {7, 8}, {7, 8}, {-1, -9}, {1, -20}, {0, -20}, {-2, -15},
        {-8, 1}, {-2, 8}, {2, 13}, {7, 11}, {2, 8}, {-2, -1}, {1, 1}, {-1, -5},
        {-6, -6}, {-5, -4}, {4, 4}, {3, 11}, {-6, 20}, {1, 13}, {-5, 9}, {0, 9},
        {-6, -14}, {-8, -11}, {-8, -6}, {-1, 3}, {5, 9}, {-3, 14}, {-5, 11}, {-15, 14},
    },
    {
        {1, 2}, {-16, 1}, {-5, -8}, {-8, -14}, {2, -4}, {-10, -5}, {-9, 1}, {-15, 2},
        {-6, 9}, {3, 15}, {1, -1}, {3, -23}, {2, -15}, {-10, -20}, {-7, -15}, {-4, -14},
        {2, 7}, {0, 13}, {8, 10}, {6, 3}, {-1, 0}, {10, -6}, {4, -9}, {-3, -14},
        {-1, 7}, {-1, 11}, {1, 4}, {5, 8}, {-2, 5}, {1, 2}, {0, 8}, {-3, -1},
        {-1, 7}, {-3, -1}, {-4, -8}, {-4, -4}, {-6, 1}, {-2, 9}, {-6, 10}, {-2, 6},
        {-3, -8}, {-9, -10}, {-8, -10}, {-4, -4}, {1, -2}, {-7, 1}, {-9, 6}, {-19, 7},
    },
    {
        {-3, -18}, {-11, -9}, {-2, -10}, {-8, -20}, {-10, -30}, {-14, -8}, {-8, -5}, {-13, -14},
        {-14, 0}, {-5, -1}, {-7, -14}, {-6, -35}, {1, -35}, {-15, -54}, {-13, -21}, {0, -25},
        {-2, 12}, {-1, 13}, {-1, 1}, {-5, -13}, {-2, -19}, {2, -30}, {1, -17}, {0, -15},
        {-4, 2}, {-4, 11}, {4, 6}, {-1, -5}, {4, 15}, {-1, 1}, {-3, 8}, {1, 0},
        {-11, -5}, {-8, -4}, {-3, 2}, {-9, 0}, {-1, 12}, {3, 12}, {2, 12}, {-2, 11},
        {-1, 2}, {-6, -10}, {0, 0}, {-4, -6}, {7, 1}, {-1, 8}, {-4, 15}, {-13, 14},
    },
    {
        {0, -13}, {-10, -3}, {-2, -14}, {-8, -19}, {-9, -26}, {-13, -6}, {-8, -3}, {-12, -10},
        {-13, 1}, {-1, -2}, {-9, -14}, {-4, -33}, {4, -32}, {-12, -49}, {-10, -20}, {-2, -23},
        {-1, 7}, {4, 14}, {1, 3}, {-3, -12}, {-2, -16}, {3, -25}, {-1, -19}, {1, -16},
        {-5, 5}, {-3, 11}, {3, 9}, {2, -1}, {1, 11}, {-3, -1}, {-1, 9}, {0, -1},
        {-8, 3}, {-8, -4}, {-2, 6}, {-8, -3}, {-5, 4}, {2, 10}, {1, 16}, {-2, 12},
        {-1, 4}, {-8, -10}, {0, -5}, {-6, -7}, {0, -5}, {-4, 1}, {-4, 13}, {-10, 18},
    },
};

template<Color C, PieceType Type, unsigned Offset = 0>
Score evaluate_king_relative(const Score* psqt, unsigned flip)
{
    Score r{0, 0};
    BitBoard pieces = position.type_bb[Type] & position.color_bb[C];
    while (pieces)
    {
        Square sq = pop(pieces);
        r += psqt[(sq ^ flip) - Offset];
    }
    return r;
}

template<Color C>
Score evaluate_king_relative()
{
    Square king_sq = first_square(position.type_bb[KING] & position.color_bb[C]);
    unsigned flip = (C == WHITE ? 0 : 56) | ((king_sq & 4) ? 7 : 0);

    unsigned rel_king_sq = king_sq ^ flip;
    unsigned king_idx = ((rel_king_sq & 48) >> 2) | (rel_king_sq & 3);

    Score r{0, 0};
    r += evaluate_king_relative<C, PAWN, 8>(king_relative_pawn_psqt[king_idx], flip);
    r += evaluate_king_relative<C, KNIGHT>(king_relative_knight_psqt[king_idx], flip);
    r += evaluate_king_relative<~C, PAWN, 8>(king_relative_opp_pawn_psqt[king_idx / 2], flip);
    return r;
}

constexpr int SCORE_MATE = 32767;
constexpr int SCORE_WIN = 32000;

struct alignas(4) EvalCache
{
    std::uint16_t key;
    std::int16_t value;
};

constexpr int EVAL_CACHE_SIZE = 1 << 12;
constexpr std::uint64_t EVAL_CACHE_MASK = EVAL_CACHE_SIZE - 1;
alignas(64) static EvalCache eval_cache[EVAL_CACHE_SIZE];

int evaluate(int alpha = -SCORE_WIN, int beta = SCORE_WIN)
{
    std::uint64_t hash_key = position.hash();
    std::uint16_t eval_key = hash_key >> 48;
    auto& ec = eval_cache[hash_key & EVAL_CACHE_MASK];
    if (ec.key == eval_key)
        return ec.value;

    Score lazy_eval = position.piece_square_values[position.next] - position.piece_square_values[~position.next];
    Score eval{};

    constexpr int lazy_threshold = 296;
    bool lazy = (lazy_eval.mid <= alpha - lazy_threshold || lazy_eval.mid >= beta + lazy_threshold) &&
            (lazy_eval.end <= alpha - lazy_threshold || lazy_eval.end >= beta + lazy_threshold);

    if (!lazy)
    {
        int pawn_index = static_cast<int>((position.type_bb[PAWN] * 0x496ea34db5092c93ULL) >> (64 - PAWN_EVAL_CACHE_SIZE_BITS));

        if (position.type_bb[PAWN] == pawn_eval_cache[pawn_index].pawns)
            eval = pawn_eval_cache[pawn_index].value;
        else
        {
            pawn_eval_cache[pawn_index].pawns = position.type_bb[PAWN];
            pawn_eval_cache[pawn_index].value = eval = evaluate_pawns<WHITE>() - evaluate_pawns<BLACK>();
        }

        Mobility mobility{};
        eval += evaluate_mobility<WHITE>(mobility) - evaluate_mobility<BLACK>(mobility);
        eval += evaluate_pieces<WHITE>(mobility) - evaluate_pieces<BLACK>(mobility);
        eval += evaluate_king_relative<WHITE>() - evaluate_king_relative<BLACK>();
    }

    Score result = Score{23, 11} + lazy_eval + (position.next == WHITE ? eval : -eval);

    constexpr int MIDGAME_WEIGHT = 40;
#ifdef TUNE
    constexpr int ENDGAME_WEIGHT = MIDGAME_WEIGHT;
#else
    constexpr int ENDGAME_WEIGHT = 36;
#endif

    int pieces = popcount(position.all_bb()) + popcount(position.all_bb() & ~position.type_bb[PAWN]) - 2;
    int v = (MIDGAME_WEIGHT * pieces * result.mid + ENDGAME_WEIGHT * (48 - pieces) * result.end) / 1920;
    if (!position.type_bb[PAWN])
        v = evaluate_pawnless(v);
    else if (popcount(position.all_bb()) <= 5 && popcount(position.type_bb[PAWN]) == 1)
        v = (position.type_bb[PAWN] & position.color_bb[WHITE]) ? evaluate_single_pawn<WHITE>(v) : evaluate_single_pawn<BLACK>(v);
    if (!lazy)
    {
        ec.key = eval_key;
        ec.value = v;
    }
    return v;
}

enum HashFlags : std::uint8_t { GEN_MASK = 0x3f, LOWER = 0x40, UPPER = 0x80 };

inline HashFlags& operator|=(HashFlags& lhs, HashFlags rhs) { return lhs = static_cast<HashFlags>(lhs | rhs); }

struct alignas(8) HashEntry
{
    std::uint16_t key;
    std::int16_t value;
    Move best_move;
    std::int8_t depth;
    HashFlags flags;

    int generation() const { return flags & GEN_MASK; }
    int get_value(int ply) const
    {
        int hv = value;
        if (hv > SCORE_WIN)
            hv -= ply;
        else if (hv < -SCORE_WIN)
            hv += ply;
        return hv;
    }
};

static HashEntry* hash_table;
static std::size_t hash_size;
static int hash_generation;

std::uint64_t hash_key(const Position& position, Move skip_move)
{
    std::uint64_t sm = skip_move ? 0x762e9afcb08cafe2ULL ^ skip_move : 0ULL;
    return position.hash() ^ sm;
}

std::size_t hash_index(std::uint64_t hk)
{
    return ((hk & 0xffffffffULL) * hash_size) >> 32;
}

int hash_score(int depth, HashFlags flags)
{
    if ((flags & (LOWER | UPPER)) == (LOWER | UPPER))
        depth += 3;
    return depth;
}

void save_hash(int value, int ply, int depth, Move mv, int alpha, int beta, Move skip_move)
{
    HashFlags flags = static_cast<HashFlags>(hash_generation);
    if (value > alpha)
        flags |= LOWER;
    if (value < beta)
        flags |= UPPER;
    if (value > SCORE_WIN)
        value += ply;
    else if (value < -SCORE_WIN)
        value -= ply;

    std::uint64_t hk = hash_key(position, skip_move);
    HashEntry& e = hash_table[hash_index(hk)];
    std::uint16_t key = static_cast<std::uint16_t>(hk >> 48);
    if (hash_score(e.depth, e.flags) <= hash_score(depth, flags) || (e.key != key && e.generation() != hash_generation))
        e = HashEntry{key, static_cast<std::int16_t>(value), mv, static_cast<std::int8_t>(depth), flags};
}

HashEntry* load_hash(Move skip_move)
{
    std::uint64_t hk = hash_key(position, skip_move);
    HashEntry* e = &hash_table[hash_index(hk)];
    if (e->key != static_cast<uint16_t>(hk >> 48) ||
            (e->best_move && !(position.color_bb[position.next] & from(e->best_move))))
        return nullptr;
    return e;
}

int hash_usage()
{
    int r = 0;
    for (int i = 0; i < 1000; i++)
    {
        if (hash_table[i].generation() == hash_generation)
            r++;
    }
    return r;
}

struct PvLine
{
    int length;
    Move moves[128];
};

alignas(64) static PvLine pv_lines[128];

void update_pv(Move best_move, int ply, bool end)
{
    pv_lines[ply].moves[0] = best_move;
    if (end)
    {
        pv_lines[ply].length = 1;
    }
    else
    {
        int len = pv_lines[ply + 1].length;
        std::copy(&pv_lines[ply + 1].moves[0], &pv_lines[ply + 1].moves[len], &pv_lines[ply].moves[1]);
        pv_lines[ply].length = 1 + len;
    }
}

using Clock = std::chrono::steady_clock;

struct Search
{
    std::istream& in;
    std::ostream& out;
    std::deque<std::string>& commands;
    Clock::duration total_time;
    Clock::duration increment;
    Clock::duration movetime;
    int moves_to_go;
    Clock::duration max_time;
    Clock::time_point start;
    long long nodes;
    unsigned int time_mask;
    int root_depth;
    int sel_depth;
    bool stopped;
    Move best_move;
    Stack* stack;

    Search(
        std::istream& i, std::ostream& o, std::deque<std::string>& cmd,
        Clock::duration time, Clock::duration inc, Clock::duration movetime, int moves_to_go, Stack* stack);

    bool is_stopped();
    bool check_stop_command();
    bool check_time(int changes, int improving);
    void set_time_mask(Clock::duration remaining);
    bool repetition(int ply) const;

    int qsearch(int ply, int depth, int alpha, int beta);
    std::pair<int, Move> search(bool pv, int ply, int depth, int alpha, int beta, Move skip_move = NULL_MOVE);
    void print_info(int depth, int score, bool upperbound);
    void iterate(int max_depth);

    Memo do_move(int ply, Move mv)
    {
        Memo memo = position.do_move(mv);
        stack[ply + 1].prev_move = mv;
        stack[ply + 1].key = position.hash();
        return memo;
    }

    void undo_move(Move mv, const Memo& memo) { position.undo_move(mv, memo); }
};

Search::Search(
    std::istream& i, std::ostream& o, std::deque<std::string>& cmd,
    Clock::duration time, Clock::duration inc, Clock::duration mt, int mtg, Stack* st)
    : in{i}, out{o}, commands{cmd}, total_time{time}, increment{inc}, movetime{mt}, moves_to_go{mtg}, max_time{mt},
    start{Clock::now()}, nodes{}, time_mask{0x1ff}, root_depth{}, sel_depth{}, stopped{}, best_move{}, stack{st}
{
    if (total_time != Clock::duration::min())
    {
        total_time -= std::min(5 * total_time / 32, Clock::duration(std::chrono::seconds(1)));
        movetime = max_time = std::min(movetime, total_time);
    }
}

bool Search::is_stopped()
{
    if (!stopped && (nodes & time_mask) == 0)
    {
        Clock::duration elapsed = Clock::now() - start;
        if (elapsed > max_time)
            stopped = true;
        else
        {
            set_time_mask(max_time - elapsed);
            if ((nodes & 0x7ffff) == 0)
            {
                if (check_stop_command())
                    stopped = true;
            }
        }
    }
    return stopped;
}

bool Search::check_stop_command()
{
    if (in.rdbuf()->in_avail())
    {
        std::string line;
        std::getline(in, line);
        std::istringstream parser{line};
        std::string token;
        parser >> token;
        if (token == "stop")
            return true;
        else if (token == "isready")
            out << "readyok" << std::endl;
        else
            commands.push_back(line);
    }
    return false;
}

bool Search::check_time(int changes, int improving)
{
    if (total_time != Clock::duration::min() && !stopped)
    {
        int pieces = popcount(position.color_bb[WHITE] | position.color_bb[BLACK]);
        max_time = std::min(
            total_time,
            20 * total_time / ((4 + ((changes <= 1) * (stack[0].singular + std::max(0, improving - 1)))) * std::min(38 + pieces, 5 * moves_to_go)) + 4 * increment);
        Clock::duration target_time = std::min(
            max_time,
            36 * total_time / ((4 + ((changes <= 1) * (stack[0].singular + std::max(0, improving - 1)))) * std::min(17 * std::max(16, pieces), 13 * moves_to_go))
                    + 22 * increment / (26 + pieces));

        Clock::duration elapsed = Clock::now() - start;
        if (elapsed > target_time)
            stopped = true;

        set_time_mask(max_time - elapsed);
    }
    return stopped;
}

void Search::set_time_mask(Clock::duration remaining)
{
    if (remaining < std::chrono::milliseconds(2))
        time_mask = 0x1ff;
    else if (remaining < std::chrono::milliseconds(5))
        time_mask = 0x3ff;
    else if (remaining < std::chrono::milliseconds(25))
        time_mask = 0x7ff;
    else if (remaining < std::chrono::milliseconds(100))
        time_mask = 0xfff;
    else if (remaining < std::chrono::milliseconds(500))
        time_mask = 0x1fff;
    else if (remaining < std::chrono::milliseconds(2000))
        time_mask = 0x3fff;
    else
        time_mask = 0x7fff;
}

bool Search::repetition(int ply) const
{
    std::uint64_t current = stack[ply].key;
    int count = 0;
    for (int i = 4; i <= position.halfmove_clock; i += 2)
    {
        if (stack[ply - i].key == current)
        {
            ++count;
            if (count >= 1 + (i >= ply))
                return true;
        }
    }
    return false;
}

std::pair<Square, PieceType> find_best_attacker(Color color, Square sq, BitBoard mask, BitBoard opp_mask)
{
    BitBoard pawns = mask & position.type_bb[PAWN] & pawn_attack[~color][sq];
    if (pawns)
        return {first_square(pawns), PAWN};
    BitBoard knights = mask & position.type_bb[KNIGHT] & knight_attack[sq];
    if (knights)
        return {first_square(knights), KNIGHT};
    BitBoard bishops = mask & position.type_bb[BISHOP] & bishop_attack(sq, mask | opp_mask);
    if (bishops)
        return {first_square(bishops), BISHOP};
    BitBoard rooks = mask & position.type_bb[ROOK] & rook_attack(sq, mask | opp_mask);
    if (rooks)
        return {first_square(rooks), ROOK};
    BitBoard queens = mask & position.type_bb[QUEEN] & queen_attack(sq, mask | opp_mask);
    if (queens)
        return {first_square(queens), QUEEN};
    BitBoard kings = mask & position.type_bb[KING] & king_attack[sq];
    if (kings)
        return {first_square(kings), KING};
    return {NO_SQUARE, KING};
}

bool see_under(Move mv, int limit)
{
    Square sq = to(mv);
    int balance = (type(mv) & CAPTURE) ? material[type(position.squares[sq])].mid : 0;

    Color att = position.next;
    BitBoard def_mask = position.color_bb[~att];
    BitBoard att_mask = position.color_bb[att] & ~from(mv);
    PieceType current = type(position.squares[from(mv)]);

    for (;;)
    {
        auto def_move = find_best_attacker(~att, sq, def_mask, att_mask);
        if (def_move.first == NO_SQUARE)
            break;
        balance -= material[current].mid;
        if (balance >= limit)
            return false;
        current = def_move.second;
        def_mask &= ~bb(def_move.first);

        auto att_move = find_best_attacker(att, sq, att_mask, def_mask);
        if (att_move.first == NO_SQUARE)
            break;
        balance += material[current].mid;
        if (balance <= limit)
            return true;
        current = att_move.second;
        att_mask &= ~bb(att_move.first);
    }

    return balance <= limit;
}

int Search::qsearch(int ply, int depth, int alpha, int beta)
{
    assert(ply < 128);
    int orig_alpha = alpha;

    Square king_sq = first_square(position.type_bb[KING] & position.color_bb[position.next]);
    BitBoard checkers = attackers(king_sq, ~position.next);

    int pat = checkers ? -SCORE_MATE + ply : evaluate(alpha, beta);
    if (pat >= beta)
        return beta;
    if (pat > alpha)
        alpha = pat;

    Move best = NULL_MOVE;

    HashEntry* he = load_hash(NULL_MOVE);
    if (he)
    {
        best = he->best_move;
        if (he->depth >= depth && (he->flags & LOWER))
        {
            int hv = he->get_value(ply);
            if (hv >= beta && hv < SCORE_MATE - ply)
                return beta;
        }
    }

    Move& counter_move = counter_moves[checkers ? counter_index(stack[ply].prev_move) : (COUNTER_MOVE_SIZE - 1)];
    MoveGen gen{checkers ? QUIETS : CAPTURES, checkers, best, counter_move, stack[ply]};

    while (Move mv = gen.next())
    {
        if (is_stopped())
            return alpha;

        if (!checkers && !(type(mv) & PROMOTION) && pat + material[type(position.squares[to(mv)])].mid < alpha - 107)
            continue;

        if (!checkers && !(type(mv) & PROMOTION) && mv != best && see_under(mv, -137))
            continue;

        ++nodes;
        Memo memo = do_move(ply, mv);
        int v;
        if (attackers(from(mv) == king_sq ? to(mv) : king_sq, position.next))
            v = -SCORE_MATE;
        else
            v = -qsearch(ply + 1, std::max(-2, depth - 1), -beta, -alpha);
        undo_move(mv, memo);

        if (v > alpha)
        {
            alpha = v;
            best = mv;
        }
        if (alpha >= beta)
            break;
    }

    if (best)
        save_hash(alpha, ply, 0, best, orig_alpha, beta, NULL_MOVE);
    return alpha;
}

template<Color C>
bool is_passed_pawn_move(Move mv)
{
    constexpr int FWD = C == WHITE ? 8 : -8;
    BitBoard all_pawns = position.type_bb[PAWN];
    BitBoard opp_pawns = all_pawns & position.color_bb[~C];
    BitBoard opp_attack = shift_signed<-FWD - 1>(opp_pawns & ~FILE_A) | shift_signed<-FWD + 1>(opp_pawns & ~FILE_H);

    return bb(to(mv)) & (C == WHITE ? 0x00ffffff00000000ULL : 0x00000000ffffff00ULL) & ~smear<-FWD>(all_pawns | opp_attack);
}

std::pair<int, Move> Search::search(bool pv, int ply, int depth, int alpha, int beta, Move skip_move)
{
    if (ply > 0)
    {
        if (position.halfmove_clock > 100)
            return {0, NULL_MOVE};
        if (beta < -SCORE_MATE + ply)
            return {beta, NULL_MOVE};
        BitBoard non_minors = position.type_bb[PAWN] | position.type_bb[ROOK] | position.type_bb[QUEEN];
        if (ply > 2 && !non_minors)
        {
            if (!position.type_bb[BISHOP] && popcount(position.type_bb[KNIGHT]) <= 2)
                return {0, NULL_MOVE};
            if (popcount(position.color_bb[WHITE]) <= 2 && popcount(position.color_bb[BLACK]) <= 2)
                return {0, NULL_MOVE};
        }
        if (ply > 2 && alpha > 0 && popcount(position.color_bb[position.next]) <= 2 &&
                !(position.color_bb[position.next] & non_minors))
            return {alpha, NULL_MOVE};
    }

    Move prev_best = NULL_MOVE;
    HashEntry* he = load_hash(skip_move);
    int hv = std::numeric_limits<int>::min();
    if (he)
    {
        hv = he->get_value(ply);
        if (he->depth >= depth + pv && position.halfmove_clock < 90)
        {
            if (hv >= beta && (he->flags & LOWER) && hv < SCORE_MATE - ply)
                return {beta, he->best_move};
            if (hv <= alpha && (he->flags & UPPER) && hv > -SCORE_MATE + ply)
                return {alpha, he->best_move};
        }
        prev_best = he->best_move;
    }

    Square king_sq = first_square(position.type_bb[KING] & position.color_bb[position.next]);
    BitBoard checkers = attackers(king_sq, ~position.next);
    int eval = checkers ? -SCORE_MATE + ply : evaluate(alpha, beta);
    if (he && ((hv > eval && (he->flags & LOWER)) || (hv < eval && (he->flags & UPPER))))
        eval = hv;

    if (!pv && !checkers && depth <= 3 && eval > beta + 187 * (depth - 1) + 44 &&
            (position.color_bb[position.next] & ~position.type_bb[KING]))
        return {beta, NULL_MOVE};

    if (ply >= sel_depth)
        sel_depth = ply + 1;
    if (pv)
        pv_lines[ply + 1].length = 0;
    stack[ply].singular = false;

    if (!pv &&
            !stack[ply].in_nmp &&
            !stack[ply - 1].in_nmp &&
            depth >= 4 &&
            eval > beta + 91 &&
            !checkers &&
            alpha > -SCORE_WIN &&
            popcount(position.color_bb[position.next] & ~position.type_bb[PAWN]) > 1)
    {
        stack[ply].in_nmp = true;
        Memo memo = do_move(ply, NULL_MOVE);

        int v = -search(false, ply + 1, depth - (13 * (eval - beta) + 340 * (depth - 1) + 3458) / 2048, -beta, -beta + 1).first;
        undo_move(NULL_MOVE, memo);
        if (v >= beta)
            v = search(false, ply, (depth - 1) / 2, beta - 1, beta).first;
        stack[ply].in_nmp = false;
        if (v >= beta)
            return {beta, NULL_MOVE};
    }

    if (!he && !checkers && eval > alpha)
        depth = std::max(1, depth - 1 - (eval > beta + 191));

    Move& counter_move = counter_moves[counter_index(stack[ply].prev_move)];
    MoveGen gen{QUIETS, checkers, prev_best, counter_move, stack[ply]};
    Move best = NULL_MOVE;
    int orig_alpha = alpha;
    Square opp_king_sq = first_square(position.type_bb[KING] & position.color_bb[~position.next]);

    int move_count = 0;
    int mcp = 0;

    while (Move mv = gen.next())
    {
        if (mv == skip_move)
        {
            gen.moves[gen.index - 1].move = NULL_MOVE;
            continue;
        }
        bool checks = is_check(mv, opp_king_sq);
        if (!checkers && !checks && move_count && depth <= 5 && eval < alpha - (210 * (depth - 1) + 61) && !(type(mv) & (CAPTURE | PROMOTION)))
            continue;
        if (!checkers && !checks && move_count && depth <= 1 && (type(mv) & CAPTURE) && !(type(mv) & PROMOTION) &&
                eval + material[type(position.squares[to(mv)])].mid < alpha - 62)
            continue;
        if (!checkers && move_count && depth <= 2 && !(type(mv) & (CAPTURE | PROMOTION)) && eval < beta - 165 * (depth - 1) + 151 &&
                see_under(mv, -137))
            continue;
        if (!checkers && move_count && depth <= 1 && (type(mv) & CAPTURE) && !(type(mv) & PROMOTION) &&
                eval < beta + 51 && see_under(mv, -137))
            continue;
        if (!checkers && !checks && move_count && depth <= 1 && !(type(mv) & (CAPTURE | PROMOTION)) && eval < alpha - 50 &&
                history[position.next][mv & FROM_TO_MASK].value < -6000)
            continue;

        if (!checkers && !checks && depth <= 7 && alpha > -SCORE_WIN && eval < alpha  - 22 &&
                !(type(mv) & (CAPTURE | PROMOTION)) && mv != prev_best &&
                popcount(position.color_bb[~position.next] & ~position.type_bb[PAWN]) > 1)
        {
            ++mcp;
            if (mcp > 2 * (depth - 1) + 4)
                break;
        }

        int extension = 0;
        if (checkers)
            extension++;
        else if (depth <= 2 && ply < 2 * root_depth && (type(mv) & CAPTURE) && (type(stack[ply].prev_move) & CAPTURE) &&
                to(mv) == to(stack[ply].prev_move) && eval + material[type(position.squares[to(mv)])].mid > alpha - 28 && eval < beta + 9)
            extension++;
        else if (!(type(mv) & CAPTURE) && type(position.squares[from(mv)]) == PAWN && ply < 2 * root_depth &&
                eval < beta + 27 &&
                (position.next == WHITE ? is_passed_pawn_move<WHITE>(mv) : is_passed_pawn_move<BLACK>(mv)) &&
                !see_under(mv, -50))
            extension++;
        else if (!skip_move && mv == prev_best &&
                he && (he->flags & LOWER) && he->value >= -SCORE_WIN &&
                depth > 4 && ply < 2 * root_depth)
        {
            int sbeta = he->value - 6 * depth;
            int sresult = search(false, ply, depth / 2, sbeta - 1, sbeta, prev_best).first;
            if (sresult < sbeta)
            {
                extension++;
                stack[ply].singular = true;
            }
            else if (sbeta >= beta)
                extension--;
        }

        int new_depth = depth + extension - 1;

        int reduction = 0;
        if (new_depth >= 3 && move_count && !checkers && !(type(mv) & (CAPTURE | PROMOTION)) &&
                mv != prev_best && !stack[ply].is_killer(mv))
        {
            reduction = move_count - 4;
            if (he && !(he->flags & LOWER))
                reduction += 4;
            if (pv)
                reduction += 1;
            reduction -= history[position.next][mv & FROM_TO_MASK].value / 1858;
            reduction = std::clamp(reduction / 4, 0, depth / 3 + (depth * move_count >= 93));
        }

        ++nodes;
        ++move_count;

        Memo memo = do_move(ply, mv);

        int v = beta;
        if (attackers(from(mv) == king_sq ? to(mv) : king_sq, position.next))
        {
            v = -SCORE_MATE;
            gen.moves[gen.index - 1].move = NULL_MOVE;
            --move_count;
        }
        else if (repetition(ply + 1))
        {
            v = 0;
        }
        else if (new_depth <= 0 && !checkers)
        {
            v = -qsearch(ply + 1, 0, -beta, -alpha);
        }
        else
        {
            if (!pv || alpha > orig_alpha)
            {
                v = -search(false, ply + 1, new_depth - reduction, -alpha - 1, -alpha).first;
                if (v > alpha && reduction)
                    v = -search(false, ply + 1, new_depth, -alpha - 1, -alpha).first;
            }
            if (v > alpha && alpha < beta - 1)
                v = -search(pv, ply + 1, new_depth, -beta, -alpha).first;
        }

        undo_move(mv, memo);

        if (is_stopped())
        {
            if (best)
                return {alpha, best};
            if (ply > 0 || best_move)
                return {0, NULL_MOVE};
        }

        if (v > alpha)
        {
            alpha = v;
            best = mv;

            if (pv && ply == 0 && alpha < beta)
                update_pv(best, ply, depth <= 1);
        }
        if (alpha >= beta)
        {
            alpha = beta;
            break;
        }
    }

    if (move_count == 0)
    {
        if (pv)
            pv_lines[ply].length = 0;
        return {skip_move ? alpha : checkers ? -SCORE_MATE + ply : 0, NO_MOVE};
    }
    if (ply > 0 && position.halfmove_clock >= 100)
        return {0, NULL_MOVE};

    assert(alpha > -SCORE_MATE);
    if (alpha >= beta)
    {
        if (!(type(best) & CAPTURE))
        {
            stack[ply].save_killer(best);
            if (stack[ply].prev_move)
                counter_move = best;
        }
        int inc = 45 * (depth - 1) + 686;
        MoveHistory* hist = history[position.next];
        for (int i = gen.index - 1; i >= 0; --i)
        {
            Move m = gen.moves[i].move;
            if (!m)
                continue;
            bool cut = (m == best);
            if (type(m) & CAPTURE)
                capture_history[6 * type(position.squares[to(m)]) + type(position.squares[from(m)])][to(m)].hit(inc, cut);
            else
                hist[m & FROM_TO_MASK].hit(inc, cut);
        }
    }
    save_hash(alpha, ply, depth, best ? best : prev_best, orig_alpha, beta, skip_move);

    if (pv && alpha > orig_alpha && alpha < beta)
        update_pv(best, ply, depth <= 1);
    return {alpha, best};
}

void Search::print_info(int depth, int score, bool upperbound)
{
    out << "info depth " << depth << " seldepth " << sel_depth << " score ";
    if (score > SCORE_WIN)
        out << "mate " << ((SCORE_MATE - score + 1) / 2);
    else if (score < -SCORE_WIN)
        out << "mate " << ((-SCORE_MATE - score) / 2);
    else
        out << "cp " << score;
    if (upperbound)
        out << " upperbound";
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(Clock::now() - start).count();
    out << " nodes " << nodes
        << std::fixed << std::setprecision(0) << " time " << (elapsed * 1000.);
    if (elapsed > 0)
        out << " nps " << (nodes / elapsed);
    out << " hashfull " << hash_usage();
    if (pv_lines[0].length > 0 && !upperbound)
    {
        out << " pv";
        for (int i = 0; i < pv_lines[0].length; i++)
            out << ' ' << pv_lines[0].moves[i];
    }
    out << std::endl;
}

void Search::iterate(int max_depth)
{
    int best_score = 0;
    int changes = 0;
    int improving = 0;
    int last_change = 0;
    pv_lines[0].length = 0;

    for (root_depth = 1; root_depth <= max_depth; ++root_depth)
    {
        std::fill_n(pawn_eval_cache, PAWN_EVAL_CACHE_SIZE, PawnEvalCache{});
        sel_depth = 0;

        int alpha = root_depth >= 4 && best_score > -SCORE_WIN ? best_score - 288 : -SCORE_MATE;
        auto v = search(true, 0, root_depth, alpha, SCORE_MATE);

        if (alpha > -SCORE_MATE && v.first <= alpha && !check_time(changes, improving))
        {
            print_info(root_depth, v.first, true);

            auto vv = search(true, 0, root_depth, -SCORE_MATE, SCORE_MATE);
            if (vv.second)
                v = vv;
        }

        if (v.second)
        {
            assert(v.first > -SCORE_MATE || v.second == NO_MOVE);
            assert(v.first < SCORE_MATE);

            if (v.second != best_move)
            {
                changes++;
                last_change = root_depth;
            }
            if (v.first > best_score)
                improving++;
            else if (v.first < best_score)
                improving = 0;
            best_score = v.first;
            best_move = v.second;
        }

        if (changes > 0 && root_depth - last_change >= 3)
            changes--;

        assert(best_move != NULL_MOVE);
        print_info(root_depth, best_score, false);

        if (check_time(changes, improving))
            break;
    }
    out << "bestmove " << best_move << std::endl;
}

constexpr char startfen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

bool get_command(std::istream& in, std::deque<std::string>& commands, std::string& line)
{
    if (commands.empty())
        return static_cast<bool>(getline(in, line));
    line = commands.front();
    commands.pop_front();
    return true;
}

void uci_main()
{
    std::string line;
    std::size_t hash_mb = 1;
    bool debug = false;
    Stack stack[256] = {};
    std::deque<std::string> commands;

    while (get_command(std::cin, commands, line))
    {
        std::istringstream parser{line};
        std::string token;
        parser >> token;

        if (token == "uci")
        {
            std::cout
                << "id name seawall " STRINGIFY(SEAWALL_VERSION) << "\n"
                << "id author petur\n"
                << "option name Hash type spin default 1 min 1 max 65536\n"
                << "uciok" << std::endl;
        }
        else if (token == "debug")
        {
            parser >> token;
            if (token == "on")
                debug = true;
            else if (token == "off")
                debug = false;
        }
        else if (token == "setoption")
        {
            parser >> token;
            parser >> token;
            for (char& c : token)
                c = std::tolower(static_cast<unsigned char>(c));
            if (token == "hash")
            {
                parser >> token;
                parser >> hash_mb;
            }
        }
        else if (token == "isready")
        {
            if (!king_attack[A1])
                init_bitboards();
            std::size_t new_hash_size = (hash_mb << 20) / sizeof(HashEntry);
            if (new_hash_size != hash_size)
            {
                delete[] hash_table;
                hash_size = new_hash_size;
                hash_table = new HashEntry[hash_size];
                std::fill_n(hash_table, hash_size, HashEntry{});
            }
            std::cout << "readyok" << std::endl;
        }
        else if (token == "ucinewgame")
        {
            if (hash_table)
                std::fill_n(hash_table, hash_size, HashEntry{});
            hash_generation = 0;
            std::fill_n(eval_cache, EVAL_CACHE_SIZE, EvalCache{});
            for (Color c : {WHITE, BLACK})
                std::fill_n(history[c], FROM_TO_SIZE, MoveHistory{});
            for (int c = PAWN; c < KING; c++)
                for (int m = PAWN; m <= KING; m++)
                    std::fill_n(capture_history[6 * c + m], 64, MoveHistory{static_cast<std::int16_t>(384 - m)});
            std::fill_n(counter_moves, COUNTER_MOVE_SIZE, NULL_MOVE);
        }
        else if (token == "position")
        {
            parser >> token;
            if (token == "startpos")
            {
                std::istringstream start{startfen};
                position.parse(start);
            }
            else
                position.parse(parser);
            std::fill_n(stack, position.halfmove_clock, Stack{});
            stack[position.halfmove_clock].key = position.hash();

            parser >> token;
            while (parser >> token)
            {
                Move mv = position.parse_move(token);
                position.do_move(mv);
                stack[position.halfmove_clock].prev_move = mv;
                stack[position.halfmove_clock].key = position.hash();
            }
            if (debug)
                position.debug(std::cout);
        }
        else if (token == "go")
        {
            int max_depth = 128;
            Clock::duration time = Clock::duration::min();
            Clock::duration inc = Clock::duration::zero();
            Clock::duration movetime = Clock::duration::max();
            int moves_to_go = 10000;

            while (parser >> token)
            {
                if (token == "depth")
                    parser >> max_depth;
                if (token == (position.next == WHITE ? "wtime" : "btime"))
                {
                    long millis;
                    parser >> millis;
                    time = std::chrono::milliseconds(millis);
                }
                if (token == (position.next == WHITE ? "winc" : "binc"))
                {
                    long millis;
                    parser >> millis;
                    inc = std::chrono::milliseconds(millis);
                }
                if (token == "movetime")
                {
                    long millis;
                    parser >> millis;
                    movetime = std::chrono::milliseconds(millis);
                }
                if (token == "movestogo")
                    parser >> moves_to_go;
            }

            hash_generation = ((hash_generation + 1) & GEN_MASK);
            Search{std::cin, std::cout, commands, time, inc, movetime, moves_to_go, &stack[position.halfmove_clock]}.iterate(max_depth);
        }
        else if (token == "quit")
        {
            break;
        }
    }
}

#ifdef TUNE
constexpr double lambda = .5;
constexpr double sig_r = 0.9948;

inline double sigmoid(double eval)
{
    return 1. / (1. + std::pow(sig_r, eval));
}

struct TuneVariable
{
    std::int16_t* p;
    double improvement;
};

class Tuner
{
public:
    void tune();
    void parse_positions();
    double evaluation_error(std::size_t offset, std::size_t step);
    bool find_best_value(TuneVariable& variable, double& best_error, int start_delta, std::size_t offset, std::size_t step);
    static void print_error(double error);

    void add_variable(Score& v);
    template<std::size_t N> void add_variables(Score (&arr)[N]);
    template<std::size_t N, std::size_t M> void add_variables(Score (&arr)[N][M]);

    template<std::size_t N> void print_variables(const char* name, Score (&arr)[N]);
    template<std::size_t N, std::size_t M> void print_variables(const char* name, Score (&arr)[N][M]);

    std::vector<std::pair<Position, double>> tuning_positions;
    std::vector<TuneVariable> variables;
};

void Tuner::parse_positions()
{
    int pos_count = 0;
    std::string line;
    while (getline(std::cin, line))
    {
        ++pos_count;
        std::istringstream parser{line};
        position.parse(parser);
        int score, result;
        parser.ignore();
        parser >> score;
        parser.ignore();
        parser >> result;

        if (!position.type_bb[PAWN] && (pos_count & 1))
            continue;
        if (popcount(position.all_bb()) <= 5)
            continue;
        if (position.fullmove_counter <= 8)
            continue;
        if (position.castling && (pos_count & 2))
            continue;
        tuning_positions.push_back({position, (1. - lambda) * result * 0.5 + lambda * sigmoid(score)});
    }

    std::cout << "Read " << pos_count << " positions, using " << tuning_positions.size() << ", discarded " << (pos_count - tuning_positions.size()) << std::endl;
}

double Tuner::evaluation_error(std::size_t offset, std::size_t step)
{
    std::fill_n(pawn_eval_cache, PAWN_EVAL_CACHE_SIZE, PawnEvalCache{});
    std::fill_n(eval_cache, EVAL_CACHE_SIZE, EvalCache{});

    double error_sum = 0.;
    for (std::size_t i = offset, sz = tuning_positions.size(); i < sz; i += step)
    {
        auto& pos = tuning_positions[i];
        position.reset();
        position.next = pos.first.next;
        position.halfmove_clock = pos.first.halfmove_clock;

        BitBoard occ = pos.first.all_bb();
        while (occ)
        {
            Square sq = pop(occ);
            position.set(sq, pos.first.squares[sq]);
        }

        double err = pos.second - sigmoid(evaluate());
        error_sum += err * err;
    }
    return error_sum * step / tuning_positions.size();
}

void Tuner::print_error(double error)
{
    std::cout << std::fixed << std::setprecision(4) << (100000. * error) << std::endl;
}

bool Tuner::find_best_value(TuneVariable& variable, double& best_error, int start_delta, std::size_t offset, std::size_t step)
{
    double start_error = best_error;
    for (int delta = start_delta; delta > 0; delta /= 2)
    {
        for (int sd : {-delta, delta})
        {
            int prev_value = *variable.p;
            *variable.p += sd;
            double error = evaluation_error(offset, step);
            if (error >= best_error)
                *variable.p = prev_value;
            else
            {
                best_error = error;
                break;
            }
        }
    }
    variable.improvement = start_error - best_error;
    print_error(best_error);
    return best_error < start_error;
}

void Tuner::add_variable(Score& v)
{
    variables.push_back({&v.mid, 0});
    variables.push_back({&v.end, 0});
}

template<std::size_t N> void Tuner::add_variables(Score (&arr)[N])
{
    for (Score& v : arr)
        add_variable(v);
}

template<std::size_t N, std::size_t M> void Tuner::add_variables(Score (&arr)[N][M])
{
    for (auto& row : arr)
        add_variables(row);
}

template<std::size_t N> void Tuner::print_variables(const char* name, Score (&arr)[N])
{
    std::cout << "Score " << name << "[" << N << "] =";
    if constexpr (N > 8)
    {
        std::cout << "\n{";
        for (std::size_t i = 0; i < N; i++)
        {
            if (i % 8 == 0)
                std::cout << "\n    ";
            std::cout << arr[i] << ", ";
        }
        std::cout << "\n};" << std::endl;
    }
    else
    {
        std::cout << " {";
        for (std::size_t i = 0; i < N; i++)
        {
            if (i > 0)
                std::cout << ", ";
            std::cout << arr[i];
        }
        std::cout << "};" << std::endl;
    }
}

template<std::size_t N, std::size_t M> void Tuner::print_variables(const char* name, Score (&arr)[N][M])
{
    std::cout << "Score " << name << "[" << N << "][" << M << "] =\n";
    std::cout << "{\n";

    for (std::size_t i = 0; i < N; i++)
    {
        if constexpr (M > 8)
        {
            std::cout << "    {";
            for (std::size_t j = 0; j < M; j++)
            {
                if (j % 8 == 0)
                    std::cout << "\n        ";
                std::cout << arr[i][j] << ", ";
            }
            std::cout << "\n    },\n";
        }
        else
        {
            std::cout << "    {";
            for (std::size_t j = 0; j < M; j++)
            {
                if (j > 0)
                    std::cout << ", ";
                std::cout << arr[i][j];
            }
            std::cout << "},\n";
        }
    }

    std::cout << "};" << std::endl;
}

void Tuner::tune()
{
    init_bitboards();
    parse_positions();

    double best_error = evaluation_error(0, 1);
    print_error(best_error);

    /*
    variables.push_back({&material[0].end, 0});
    for (int i = 1; i < 5; i++)
        add_variable(material[i]);
    add_variables(piece_square_table);
    add_variables(pawn_evals);
    add_variables(mobility_evals);
    add_variables(king_evals);
    add_variables(piece_evals);
    add_variables(king_relative_pawn_psqt);
    add_variables(king_relative_knight_psqt);
    */
    add_variables(king_relative_opp_pawn_psqt);

    std::size_t step = tuning_positions.size() / 1800000U + 1;

    for (int k = 0; k < 4; k++)
    {
        std::cout << "===== " << k << " =====" << std::endl;
        bool progress = false;
        best_error = evaluation_error(k % step, step);
        print_error(best_error);
        for (auto& p : variables)
        {
            if (find_best_value(p, best_error, 1, k % step, step))
                progress = true;
        }
        if (!progress)
            break;
        best_error = evaluation_error(0, 1);
        print_error(best_error);
        for (int n = 0; n < 4; n++)
        {
            std::cout << "===== " << k << ":" << n << " =====" << std::endl;

            std::sort(
                variables.begin(), variables.end(),
                [](const TuneVariable& lhs, const TuneVariable& rhs) { return lhs.improvement > rhs.improvement; }
            );

            double cutoff = variables.front().improvement * std::pow(2., -0.35 * n - 1.);
            for (auto& v : variables)
            {
                if (v.improvement <= cutoff)
                    break;
                find_best_value(v, best_error, 31 / (n + k + 2), 0, 1);
            }
        }
    }

    print_error(best_error);
    print_variables("material", material);
    print_variables("piece_square_table", piece_square_table);
    print_variables("pawn_evals", pawn_evals);
    print_variables("mobility_evals", mobility_evals);
    print_variables("king_evals", king_evals);
    print_variables("piece_evals", piece_evals);
    print_variables("king_relative_pawn_psqt", king_relative_pawn_psqt);
    print_variables("king_relative_knight_psqt", king_relative_knight_psqt);
    print_variables("king_relative_opp_pawn_psqt", king_relative_opp_pawn_psqt);
}
#endif

int main()
{
    std::ios::sync_with_stdio(false);
    std::cout << "# seawall " STRINGIFY(SEAWALL_VERSION) << std::endl;

#ifdef TUNE
    Tuner{}.tune();
#else
    uci_main();
#endif

    return 0;
}
