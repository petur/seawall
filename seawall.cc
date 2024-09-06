#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <deque>
#include <execution>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifdef __BMI2__
#include <x86intrin.h>
#endif

#define STRINGIFY0(x) #x
#define STRINGIFY(x) STRINGIFY0(x)

using Clock = std::chrono::steady_clock;

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

struct alignas(4) Weight
{
    std::int16_t mid;
    std::int16_t end;

    constexpr Weight() : mid{}, end{} { }
    constexpr Weight(std::int16_t m, std::int16_t e) : mid{m}, end{e} { }
};

#ifndef TUNE
inline Weight operator-(Weight w)
{
    return {static_cast<std::int16_t>(-w.mid), static_cast<std::int16_t>(-w.end)};
}

inline Weight& operator+=(Weight& lhs, Weight rhs)
{
    lhs.mid += rhs.mid;
    lhs.end += rhs.end;
    return lhs;
}

inline Weight operator+(Weight lhs, Weight rhs)
{
    return lhs += rhs;
}

inline Weight& operator-=(Weight& lhs, Weight rhs)
{
    lhs.mid -= rhs.mid;
    lhs.end -= rhs.end;
    return lhs;
}

inline Weight operator-(Weight lhs, Weight rhs)
{
    return lhs -= rhs;
}

inline Weight operator*(Weight lhs, int rhs)
{
    return {static_cast<std::int16_t>(lhs.mid * rhs), static_cast<std::int16_t>(lhs.end * rhs)};
}

using Score = Weight;
using Feature = Weight;
#else
using Feature = std::pair<Weight*, int>;

inline Feature operator*(Weight& lhs, int rhs)
{
    return {&lhs, rhs};
}

struct Score
{
    std::vector<Feature> features;

    Score() = default;
    Score(Weight& w) : features{{&w, 1}} {}
};

Score operator-(const Score& s)
{
    Score result{};
    for (auto f : s.features)
        result.features.push_back({f.first, -f.second});
    return result;
}

Score& operator+=(Score& lhs, Weight& rhs)
{
    lhs.features.push_back({&rhs, 1});
    return lhs;
}

Score& operator+=(Score& lhs, const Feature& rhs)
{
    lhs.features.push_back(rhs);
    return lhs;
}

Score& operator+=(Score& lhs, const Score& rhs)
{
    lhs.features.insert(lhs.features.end(), rhs.features.begin(), rhs.features.end());
    return lhs;
}

inline Score operator+(Score lhs, const Score& rhs)
{
    return lhs += rhs;
}

inline Score operator+(const Feature& lhs, const Feature& rhs)
{
    Score result{};
    result += lhs;
    result += rhs;
    return result;
}

inline Score operator+(Score lhs, const Feature& rhs)
{
    return lhs += rhs;
}

inline Score operator+(Score lhs, Weight& rhs)
{
    return lhs += rhs;
}

inline Score operator+(Weight& lhs, Weight& rhs)
{
    Score result{};
    result += lhs;
    result += rhs;
    return result;
}

inline Score& operator-=(Score& lhs, const Score& rhs)
{
    for (auto f : rhs.features)
        lhs.features.push_back({f.first, -f.second});
    return lhs;
}

inline Score operator-(Score lhs, const Score& rhs)
{
    return lhs -= rhs;
}
#endif

std::ostream& operator<<(std::ostream& out, Weight w)
{
    return out << '{' << w.mid << ", " << w.end << '}';
}

#ifndef TUNE
constexpr
#endif
Weight material[6] = {{102, 125}, {358, 423}, {362, 464}, {427, 774}, {1022, 1430}, {2000, 2000}};

alignas(64)
#ifndef TUNE
constexpr
#endif
Weight piece_square_table[6][64] =
{
    {
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
        {-6, 0}, {-2, 0}, {-8, 4}, {-9, 10}, {-1, 5}, {8, 1}, {7, 0}, {-7, -5},
        {-8, -3}, {-9, -1}, {-5, -1}, {-3, -2}, {-1, -2}, {-1, 0}, {4, -6}, {-5, -8},
        {-3, 0}, {-1, 2}, {0, -10}, {4, -26}, {11, -24}, {3, -5}, {6, 0}, {-1, -5},
        {-6, 27}, {0, 25}, {-4, 7}, {5, -5}, {7, -1}, {13, -2}, {0, 25}, {2, 23},
        {35, 88}, {48, 67}, {49, 60}, {62, 40}, {56, 51}, {83, 51}, {62, 67}, {37, 83},
        {149, 130}, {88, 149}, {75, 149}, {96, 143}, {100, 144}, {84, 143}, {84, 151}, {115, 154},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    },
    {
        {-31, -50}, {-21, -53}, {-14, -33}, {-18, -28}, {-6, -34}, {-13, -25}, {-19, -36}, {-37, -53},
        {-17, -54}, {-12, -14}, {-12, -2}, {2, -3}, {-2, -4}, {4, -3}, {-7, -11}, {-8, -38},
        {-23, -35}, {-2, -13}, {0, 16}, {15, 12}, {14, 18}, {0, 13}, {-1, -5}, {-14, -23},
        {0, -17}, {8, 5}, {27, 23}, {27, 32}, {26, 34}, {34, 30}, {19, 8}, {0, -3},
        {9, 3}, {6, 7}, {40, 27}, {55, 27}, {42, 37}, {46, 27}, {18, 15}, {18, 0},
        {-13, 4}, {32, 2}, {58, 16}, {66, 7}, {67, 5}, {102, 0}, {33, 6}, {3, -3},
        {-45, -2}, {-12, 6}, {21, 1}, {46, 6}, {34, 3}, {57, -20}, {-7, -5}, {-18, -17},
        {-119, -20}, {-33, 0}, {-16, 5}, {-1, 0}, {12, -2}, {-42, 13}, {-43, -6}, {-73, -55},
    },
    {
        {9, -49}, {8, -34}, {-3, -24}, {-24, -11}, {-18, -13}, {-22, -14}, {-18, -28}, {-10, -39},
        {15, -32}, {18, -20}, {9, -11}, {-2, -1}, {2, -3}, {16, -14}, {9, -4}, {-3, -44},
        {3, -24}, {15, 6}, {12, 24}, {9, 27}, {9, 30}, {12, 21}, {17, -2}, {-2, -23},
        {-6, -13}, {3, 11}, {8, 32}, {29, 36}, {20, 33}, {8, 25}, {2, 7}, {3, -17},
        {-12, -6}, {0, 19}, {11, 25}, {48, 23}, {28, 37}, {20, 24}, {13, 13}, {-4, -13},
        {-26, 6}, {6, 15}, {6, 22}, {34, 10}, {36, 13}, {54, 14}, {32, 12}, {-3, 1},
        {-72, 25}, {-47, 31}, {-10, 14}, {-26, 22}, {-38, 30}, {-13, 20}, {-28, 23}, {-55, 5},
        {-56, 19}, {-48, 25}, {-77, 32}, {-59, 29}, {-20, 14}, {-76, 32}, {-16, 11}, {-35, 15},
    },
    {
        {-3, -5}, {-4, -5}, {-1, -2}, {1, 0}, {5, -7}, {4, -3}, {-15, -1}, {-12, -17},
        {-26, -4}, {-22, -4}, {-10, -1}, {-8, -1}, {-2, -4}, {4, -9}, {-13, -13}, {-71, -3},
        {-31, 14}, {-29, 18}, {-26, 21}, {-25, 17}, {-15, 11}, {-25, 16}, {-16, 7}, {-28, 2},
        {-31, 35}, {-42, 45}, {-25, 40}, {-25, 37}, {-26, 34}, {-25, 36}, {-31, 36}, {-24, 18},
        {-16, 45}, {-13, 48}, {6, 38}, {12, 33}, {-1, 39}, {6, 39}, {-18, 44}, {-6, 29},
        {6, 45}, {34, 32}, {37, 31}, {49, 23}, {60, 19}, {70, 18}, {45, 29}, {9, 35},
        {40, 36}, {39, 37}, {85, 20}, {110, 12}, {72, 24}, {94, 13}, {43, 31}, {47, 25},
        {89, 7}, {81, 16}, {84, 14}, {81, 15}, {61, 20}, {66, 20}, {44, 31}, {73, 19},
    },
    {
        {-8, -64}, {-1, -77}, {4, -86}, {2, -63}, {-2, -84}, {-18, -106}, {-23, -89}, {-18, -60},
        {-3, -59}, {2, -42}, {9, -42}, {2, -30}, {8, -42}, {15, -57}, {21, -90}, {-18, -59},
        {-7, -32}, {0, -19}, {-9, 3}, {-16, 6}, {-12, 13}, {-5, 8}, {3, 0}, {-4, -28},
        {-8, -25}, {-21, 10}, {-25, 34}, {-30, 60}, {-20, 46}, {-13, 39}, {0, 20}, {-15, 28},
        {-17, 0}, {-27, 30}, {-24, 49}, {-22, 68}, {-17, 84}, {-10, 72}, {-13, 71}, {4, 29},
        {-29, 3}, {-11, 16}, {-19, 49}, {-3, 62}, {14, 70}, {55, 68}, {28, 61}, {0, 67},
        {-51, 31}, {-49, 42}, {-4, 55}, {-2, 64}, {4, 69}, {40, 45}, {17, 50}, {-1, 50},
        {-8, 21}, {49, 7}, {48, 27}, {41, 38}, {60, 40}, {74, 32}, {74, 17}, {53, 34},
    },
    {
        {-39, 0}, {6, -4}, {21, -32}, {-31, -18}, {-2, -39}, {-8, -23}, {3, 0}, {1, -11},
        {12, -5}, {-3, 12}, {-1, 0}, {-12, -4}, {-12, -3}, {-1, -1}, {-6, 12}, {1, 0},
        {10, -8}, {20, -1}, {2, 0}, {0, -2}, {-3, 1}, {-4, 0}, {3, 5}, {-5, -1},
        {8, -9}, {26, -8}, {37, -13}, {20, -5}, {-1, -3}, {-11, 0}, {0, -6}, {-32, -5},
        {6, 1}, {24, 7}, {0, 0}, {-7, 2}, {-3, 1}, {-25, 3}, {-27, 24}, {-45, 10},
        {19, -9}, {22, 1}, {11, -9}, {-1, -3}, {9, -4}, {-10, -1}, {-16, 10}, {-3, -4},
        {18, -4}, {29, 13}, {7, 5}, {18, -2}, {5, 0}, {-8, 10}, {-14, 27}, {-12, 5},
        {1, -9}, {8, 8}, {53, -5}, {-10, 8}, {9, -2}, {-3, 8}, {-7, 11}, {-11, -20},
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

#ifndef TUNE
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
#endif

#ifndef TUNE
constexpr
#endif
Weight pawn_evals[4][15] =
{
    {
        {-17, 0}, {10, 7}, {8, -15}, {-2, -36}, {34, 78}, {-15, 19}, {42, 14}, {24, 30},
        {-30, 32}, {10, 18}, {23, 37}, {16, -43}, {46, -66}, {10, 35}, {4, 0},
    },
    {
        {-1, -2}, {7, 0}, {0, -17}, {-2, -33}, {6, 79}, {-1, 0}, {30, 5}, {29, 22},
        {-25, 48}, {11, 7}, {9, 42}, {-3, -35}, {3, -30}, {8, 33}, {7, -9},
    },
    {
        {0, 2}, {5, 1}, {-7, -15}, {-14, -21}, {30, 66}, {-10, -11}, {21, 20}, {24, 25},
        {0, 49}, {10, 11}, {20, 32}, {-12, -19}, {-18, -12}, {12, 30}, {-10, -4},
    },
    {
        {-3, 3}, {-1, 20}, {-19, -8}, {-23, 8}, {28, 75}, {-5, -30}, {10, 55}, {26, 44},
        {12, 44}, {7, 29}, {17, 50}, {-21, 6}, {-19, 2}, {5, 36}, {-7, -8},
    },
};

template<Color C>
Score evaluate_pawns()
{
    constexpr int FWD = C == WHITE ? 8 : -8;
    BitBoard all_pawns = position.type_bb[PAWN];
    if (!all_pawns)
        return Score{};

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

#ifndef TUNE
    const
#endif
    Weight* pe = pawn_evals[(popcount(all_pawns) - 1) / 4];
    Score r{};

    if (unmoved_pawns)
    {
        r += pe[0] * popcount(unmoved_pawns)
                + pe[1] * popcount(unmoved_pawns & adjacent)
                + pe[2] * popcount(unmoved_pawns & own_blocked)
                + pe[3] * popcount(unmoved_pawns & doubled)
                + pe[4] * popcount(unmoved_pawns & opp_blocked)
                + pe[5] * popcount(unmoved_pawns & backwards);
    }
    if (moved_pawns)
    {
        r += pe[6] * popcount(moved_pawns & CP_RANKS1 & own_attack)
                + pe[7] * popcount(moved_pawns & CP_RANKS2 & own_attack)
                + pe[8] * popcount(moved_pawns & PP_RANKS & ~smear<-FWD>(opp_pawns | opp_attack))
                + pe[9] * popcount(moved_pawns & NEAR_RANKS & adjacent)
                + pe[10] * popcount(moved_pawns & FAR_RANKS & adjacent)
                + pe[11] * popcount(moved_pawns & own_blocked)
                + pe[12] * popcount(moved_pawns & doubled)
                + pe[13] * popcount(moved_pawns & BP_RANKS & opp_blocked)
                + pe[14] * popcount(moved_pawns & BP_RANKS & backwards);
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

int evaluate_pawnless(const Position& pos, int v)
{
    int piece_count = popcount(pos.all_bb());
    if (piece_count <= 4 &&
            !(pos.type_bb[PAWN] | pos.type_bb[ROOK] | pos.type_bb[QUEEN]))
    {
        if (popcount(pos.color_bb[WHITE]) <= 2 && popcount(pos.color_bb[BLACK]) <= 2)
            return 0;
        if (!pos.type_bb[BISHOP])
            return 0;
    }

    Square own_king_sq = first_square(pos.type_bb[KING] & pos.color_bb[pos.next]);
    Square opp_king_sq = first_square(pos.type_bb[KING] & pos.color_bb[~pos.next]);
    int king_value = pawnless_king_table[own_king_sq] - pawnless_king_table[opp_king_sq];

    if (piece_count == 4 && (pos.type_bb[ROOK] & pos.color_bb[WHITE]) && (pos.type_bb[ROOK] & pos.color_bb[BLACK]))
    {
        v /= 32;
        v += king_value / 8;
        v += 1;
    }
    else if ((!(pos.color_bb[WHITE] & ~pos.type_bb[KING]) || !(pos.color_bb[BLACK] & ~pos.type_bb[KING])) &&
            (pos.type_bb[QUEEN] || pos.type_bb[ROOK]))
    {
        v = material[QUEEN].end + popcount(pos.type_bb[ROOK]) * material[ROOK].end + popcount(pos.type_bb[QUEEN]) * material[QUEEN].end;
        if (!(pos.color_bb[pos.next] & ~pos.type_bb[KING]))
            v = -v;
        v += king_value + 16;
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

alignas(64) constexpr std::uint64_t kpk_bitbase[] =
{
    0xc0c0c0c0c0c0fcfc, 0xc0c0c0c0c0c0f8f8, 0xc0c0c0c0c0c0f0f1, 0xc0c0c0c0c0c0e2e3, 0xc0c0c0c0c0c0c6c7, 0xc0c0c0c0c0c08e8f,
    0xc0c0c0c0c0c01e1f, 0xc0c0c0c0c0c03e3f, 0x0000000000000000, 0xc0c0c0c0c0f8f8f8, 0xc0c0c0c0c0f0f0f1, 0xc0c0c0c0c0e0e2e3,
    0xc0c0c0c0c0c0c6c7, 0xc0c0c0c0c0808e8f, 0xc0c0c0c0c0001e1f, 0xc0c0c0c0c0003e3f, 0xc0c0c0c0f8f8fcff, 0xc0c0c0c0f8f8f8ff,
    0xc0c0c0c0f0f0f0ff, 0xc0c0c0c0e0e0e2ff, 0xc0c0c0c0c0c0c6ff, 0xc0c0c0c080808eff, 0xc0c0c0c000001eff, 0xc0c0c0c000003eff,
    0xe0e0e0f8f8fcfeff, 0xe0e0e0f8f8f8feff, 0xe0e0e0f0f0f0feff, 0xe0e0e0e0e0e0feff, 0xe0e0e0c0c0c0feff, 0xc0c0c0808080feff,
    0xc0c0c0000000feff, 0xc0c0c0000000feff, 0xf0f0f8f8f8fcfeff, 0xf0f0f8f8f8fcfeff, 0xf0f0f0f0f0fcfeff, 0xf0f0e0e0e0fcfeff,
    0xe0e0c0c0c0f8feff, 0xc0c0808080f0feff, 0xc0c0000000e0feff, 0xc0c0000000c0feff, 0xf8f8f8f8f8fcfeff, 0xf8f8f8f8fcfcfeff,
    0xf8f0f0f0f8fcfeff, 0xf0e0e0e0f0fcfeff, 0xe0c0c0c0e0f8feff, 0xc0808080c0f0feff, 0xc0000000c0e0feff, 0xc0000000c0c0feff,
    0xf8f8f8f8f8f8feff, 0xf8f8f8f8f8f8feff, 0xf0f0f0f8f8f8feff, 0xe0e0e0f0f0f0feff, 0xc0c0c0e0e0e0feff, 0x808080c0c0c0feff,
    0x000000c0c0c0feff, 0x000000c0c0c0feff, 0xf8f8f0f0f0f0feff, 0xf8f8f0f0f0f0feff, 0xf0f0f0f0f0f0feff, 0xe0e0f0f0f0f0feff,
    0xc0c0e0e0e0e0feff, 0x8080c0c0c0c0feff, 0x0000c0c0c0c0feff, 0x0000c0c0c0c0feff, 0x80808080808080fc, 0x80808080808080f8,
    0x80808080808080f0, 0x80808080808080e0, 0x80808080808080c4, 0x808080808080808c, 0x808080808080001c, 0x808080808080003c,
    0x0000000000000000, 0x808080808080f8f8, 0x808080808080f0f0, 0x808080808080e0e0, 0x808080808080c0c4, 0x808080808080808c,
    0x808080808000001c, 0x808080808000003c, 0x8080808080f0f0ff, 0x8080808080f8f8ff, 0x8080808080f0f0fc, 0x8080808080e0e0fc,
    0x8080808080c0c0fc, 0x80808080808080fc, 0x80808080000000fc, 0x80808080000000fc, 0xc0c0c0c0f0f0fcfc, 0xc0c0c0c0f8f8fcfc,
    0xc0c0c0c0f0f0fcfc, 0xc0c0c0c0e0e0fcfc, 0xc0c0c0c0c0c0f0fc, 0x808080808080e0fc, 0x808080000000c0fc, 0x80808000000080fc,
    0xe0e0e0f0f0f0fcfc, 0xe0e0e0f8f8fcfcfc, 0xe0e0e0f0f0fcfcfc, 0xe0e0e0e0e0f8fcfc, 0xc0c0c0c0c0f0f0fc, 0x8080808080e0e0fc,
    0x8080000000c0c0fc, 0x80800000008080fc, 0xf0f0f0f0f0f0fcfc, 0xf0f0f8f8f8f8fcfc, 0xf0f0f0f0f0f0fcfc, 0xe0e0e0e0e0e0fcfc,
    0xc0c0c0c0c0c0f0fc, 0x808080808080e0fc, 0x800000008080c0fc, 0x80000000808080fc, 0xf0f0f0f0f0f0f0fc, 0xf8f8f0f0f0f0f0fc,
    0xf0f0f0f0f0f0f0fc, 0xe0e0e0e0e0e0e0fc, 0xc0c0c0c0c0c0c0fc, 0x80808080808080fc, 0x00000080808080fc, 0x00000080808080fc,
    0xf0e0e0e0e0e0e0fc, 0xf8e0e0e0e0e0e0fc, 0xf0e0e0e0e0e0e0fc, 0xe0e0e0e0e0e0e0fc, 0xc0c0c0c0c0c0c0fc, 0x80808080808080fc,
    0x00008080808080fc, 0x00008080808080fc, 0xfff8f8f8f8f8fcfc, 0xfff8f8f8f8f8f8f8, 0xfff0f0f0f0f0f1f1, 0xfff0f0f0f0f0e1e3,
    0xffe0e0e0e0e0c5c7, 0xffc0c0c0c0c08d8f, 0xe080808080801d1f, 0xc080808080803d3f, 0xfffffcfffcf8fcfc, 0x0000000000000000,
    0xfffff9fff9f0f1f1, 0xfffff1fdf1e0e1e3, 0xffffe0f8e0c0c5c7, 0xfff0c0f0c0808d8f, 0xe0e080e080001d1f, 0xc0c080c080003d3f,
    0xfffffffefcf8fcff, 0xfffffffdf8f8f8ff, 0xfffffffbf1f0f1ff, 0xfffffff5e1e0e1ff, 0xfffff8e8c0c0c5ff, 0xfff0f0d080808dff,
    0xe0e0e0a000001dff, 0xc0c0c0c000003dff, 0xfffffffcfcf8fdff, 0xfffffff8f8f8fdff, 0xfffffff1f1f0fdff, 0xffffffe1e1e0fdff,
    0xfffff8c0c0c0fdff, 0xfff0f0808080fdff, 0xe0e0e0000000fdff, 0xc0c0c0000000fdff, 0xfffffcfcfcf8fdff, 0xfffff8f8f8f8fdff,
    0xfffff1f1f1f8fdff, 0xffffe3e1e1f8fdff, 0xffffc0c0c0f8fdff, 0xfff0808080f0fdff, 0xe0e0000000e0fdff, 0xc0c0000000c0fdff,
    0xfffcfcfcf8f8fdff, 0xfff8f8f8f8f8fdff, 0xfff1f1f1f8f8fdff, 0xffe3e1e1f0f8fdff, 0xffc7c0c0e0f8fdff, 0xff808080c0f0fdff,
    0xe000000080e0fdff, 0xc000000080c0fdff, 0xfcfcfcf0f0f0fdff, 0xf8f8f8f0f0f0fdff, 0xf1f1f1f0f0f0fdff, 0xe3e1e1f0f0f0fdff,
    0xc7c0c0e0e0e0fdff, 0x8f8080c0c0c0fdff, 0x000000808080fdff, 0x000000808080fdff, 0xfcfce0e0e0e0fdff, 0xf8f8e0e0e0e0fdff,
    0xf1f1e0e0e0e0fdff, 0xe1e1e0e0e0e0fdff, 0xc0c0e0e0e0e0fdff, 0x8080c0c0c0c0fdff, 0x000080808080fdff, 0x000080808080fdff,
    0xf0f0f0f0f0f0f8fc, 0xf0f0f0f0f0f0f8f8, 0xe0e0e0e0e0e0e0f1, 0xe0e0e0e0e0e0e0e0, 0xc0c0c0c0c0c0c0c0, 0x8080808080808088,
    0x0000000000000018, 0x0000000000000038, 0xfff8f8f8f8f8fcfc, 0x0000000000000000, 0xfff0f0f0f0f0f1f1, 0xffe0e0e0e0e0e0e0,
    0xffc0c0c0c0c0c0c0, 0xe080808080808088, 0xc000000000000018, 0x8000000000000038, 0xfffffcfdfcfcfcff, 0xfffff8faf8f8f8ff,
    0xfffff1f5f1f1f1ff, 0xffffe0e8e0e0e0f8, 0xfff0c0d0c0c0c0f8, 0xe0e080a0808080f8, 0xc0c00040000000f8, 0x80808080000000f8,
    0xfffffffcfcf8f8f8, 0xfffffff8f8f8f8f8, 0xfffffff1f1f0f8f8, 0xfffff8e0e0e0f8f8, 0xfff0f0c0c0c0f8f8, 0xe0e0e0808080e0f8,
    0xc0c0c0000000c0f8, 0x80808000000080f8, 0xfffffcfcf8f8f8f8, 0xfffff8f8f8f8f8f8, 0xfffff1f1f0f8f8f8, 0xffffe0e0e0f8f8f8,
    0xfff0c0c0c0f0f8f8, 0xe0e0808080e0e0f8, 0xc0c0000000c0c0f8, 0x80800000008080f8, 0xfffcfcf0f0f0f8f8, 0xfff8f8f0f0f0f8f8,
    0xfff1f1f0f0f0f8f8, 0xffe0e0e0e0e0f8f8, 0xffc0c0c0c0c0f8f8, 0xe08080808080e0f8, 0xc00000000000c0f8, 0x80000000000080f8,
    0xfcfce0e0e0e0e0f8, 0xf8f8e0e0e0e0e0f8, 0xf1f1e0e0e0e0e0f8, 0xe0e0e0e0e0e0e0f8, 0xc0c0c0c0c0c0c0f8, 0x80808080808080f8,
    0x00000000000000f8, 0x00000000000000f8, 0xfcc0c0c0c0c0c0f8, 0xf8c0c0c0c0c0c0f8, 0xf1c0c0c0c0c0c0f8, 0xe0c0c0c0c0c0c0f8,
    0xc0c0c0c0c0c0c0f8, 0x80808080808080f8, 0x00000000000000f8, 0x00000000000000f8, 0xfff0f0f0f0f0f8fc, 0xfff0f0f0f0f0f8f8,
    0xfff1f1f1f1f1f1f1, 0xffe1e1e1e1e1e3e3, 0xffe1e1e1e1e1c3c7, 0xffc1c1c1c1c18b8f, 0xff80808080801b1f, 0xc000000000003b3f,
    0xfffff8fbf8f0f8fc, 0xfffff9fff9f0f8f8, 0x0000000000000000, 0xfffff3fff3e1e3e3, 0xffffe3fbe3c1c3c7, 0xffffc1f1c1818b8f,
    0xffe080e080001b1f, 0xc0c000c000003b3f, 0xfffffffaf8f0f8ff, 0xfffffffdf8f0f8ff, 0xfffffffbf1f1f1ff, 0xfffffff7e3e1e3ff,
    0xffffffebc3c1c3ff, 0xfffff1d181818bff, 0xffe0e0a000001bff, 0xc0c0c04000003bff, 0xfffffff8f8f0fbff, 0xfffffff8f8f0fbff,
    0xfffffff1f1f1fbff, 0xffffffe3e3e1fbff, 0xffffffc3c3c1fbff, 0xfffff1818181fbff, 0xffe0e0000000fbff, 0xc0c0c0000000fbff,
    0xfffffcf8f8f1fbff, 0xfffff8f8f8f1fbff, 0xfffff1f1f1f1fbff, 0xffffe3e3e3f1fbff, 0xffffc7c3c3f1fbff, 0xffff818181f1fbff,
    0xffe0000000e0fbff, 0xc0c0000000c0fbff, 0xfffcf8f8f0f1fbff, 0xfff8f8f8f1f1fbff, 0xfff1f1f1f1f1fbff, 0xffe3e3e3f1f1fbff,
    0xffc7c3c3e1f1fbff, 0xff8f8181c1f1fbff, 0xff00000080e0fbff, 0xc000000000c0fbff, 0xfcf8f8e0e0e0fbff, 0xf8f8f8e0e0e0fbff,
    0xf1f1f1e0e0e0fbff, 0xe3e3e3e0e0e0fbff, 0xc7c3c3e0e0e0fbff, 0x8f8181c0c0c0fbff, 0x1f0000808080fbff, 0x000000000000fbff,
    0xf8f8c0c0c0c0fbff, 0xf8f8c0c0c0c0fbff, 0xf1f1c0c0c0c0fbff, 0xe3e3c0c0c0c0fbff, 0xc3c3c0c0c0c0fbff, 0x8181c0c0c0c0fbff,
    0x000080808080fbff, 0x000000000000fbff, 0xe0e0e0e0e0e0f0f0, 0xe0e0e0e0e0e0f0f8, 0xe0e0e0e0e0e0f1f1, 0xc0c0c0c0c0c0c1e3,
    0xc0c0c0c0c0c0c1c1, 0x8080808080808181, 0x0000000000000011, 0x0000000000000031, 0xfff0f0f0f0f0f0f0, 0xfff0f0f0f0f0f8f8,
    0x0000000000000000, 0xffe1e1e1e1e1e3e3, 0xffc1c1c1c1c1c1c1, 0xff80808080808181, 0xc000000000000011, 0x8000000000000031,
    0xfffff0f1f0f0f0f1, 0xfffff8faf8f8f8ff, 0xfffff1f5f1f1f1ff, 0xffffe3ebe3e3e3ff, 0xffffc1d1c1c1c1f1, 0xffe080a0808081f1,
    0xc0c00040000000f1, 0x80800080000000f1, 0xfffff1f0f0f0f1f1, 0xfffffff8f8f0f1f1, 0xfffffff1f1f1f1f1, 0xffffffe3e3e1f1f1,
    0xfffff1c1c1c1f1f1, 0xffe0e0808080f1f1, 0xc0c0c0000000c0f1, 0x80808000000080f1, 0xfffff0f0f0f1f1f1, 0xfffff8f8f0f1f1f1,
    0xfffff1f1f1f1f1f1, 0xffffe3e3e1f1f1f1, 0xffffc1c1c1f1f1f1, 0xffe0808080e0f1f1, 0xc0c0000000c0c0f1, 0x80800000008080f1,
    0xfff0f0e0e0e0f1f1, 0xfff8f8e0e0e0f1f1, 0xfff1f1e0e0e0f1f1, 0xffe3e3e0e0e0f1f1, 0xffc1c1c0c0c0f1f1, 0xff8080808080f1f1,
    0xc00000000000c0f1, 0x80000000000080f1, 0xf0f0c0c0c0c0c0f1, 0xf8f8c0c0c0c0c0f1, 0xf1f1c0c0c0c0c0f1, 0xe3e3c0c0c0c0c0f1,
    0xc1c1c0c0c0c0c0f1, 0x80808080808080f1, 0x00000000000000f1, 0x00000000000000f1, 0xf0808080808080f1, 0xf8808080808080f1,
    0xf1808080808080f1, 0xe3808080808080f1, 0xc1808080808080f1, 0x80808080808080f1, 0x00000000000000f1, 0x00000000000000f1,
    0xffe0e0e0e0e0f4fc, 0xffe1e1e1e1e1f0f8, 0xffe1e1e1e1e1f1f1, 0xffe3e3e3e3e3e3e3, 0xffc3c3c3c3c3c7c7, 0xffc3c3c3c3c3878f,
    0xff8383838383171f, 0xff0101010101373f, 0xffffe0e3e0e0f4fc, 0xfffff1f7f1e0f0f8, 0xfffff3fff3e1f1f1, 0x0000000000000000,
    0xffffe7ffe7c3c7c7, 0xffffc7f7c783878f, 0xffff83e38303171f, 0xffc101c10101373f, 0xffffe3e2e0e0f4ff, 0xfffffff5f0e0f0ff,
    0xfffffffbf1e1f1ff, 0xfffffff7e3e3e3ff, 0xffffffefc7c3c7ff, 0xffffffd7878387ff, 0xffffe3a3030317ff, 0xffc1c141010137ff,
    0xffffe3e0e0e0f7ff, 0xfffffff0f0e0f7ff, 0xfffffff1f1e1f7ff, 0xffffffe3e3e3f7ff, 0xffffffc7c7c3f7ff, 0xffffff878783f7ff,
    0xffffe3030303f7ff, 0xffc1c1010101f7ff, 0xffffe0e0e0e3f7ff, 0xfffff8f0f0e3f7ff, 0xfffff1f1f1e3f7ff, 0xffffe3e3e3e3f7ff,
    0xffffc7c7c7e3f7ff, 0xffff8f8787e3f7ff, 0xffff030303e3f7ff, 0xffc1010101c1f7ff, 0xfffce0e0e0e3f7ff, 0xfff8f0f0e1e3f7ff,
    0xfff1f1f1e3e3f7ff, 0xffe3e3e3e3e3f7ff, 0xffc7c7c7e3e3f7ff, 0xff8f8787c3e3f7ff, 0xff1f030383e3f7ff, 0xff01010101c1f7ff,
    0xfce0e0c0c0c0f7ff, 0xf8f0f0c1c1c1f7ff, 0xf1f1f1c1c1c1f7ff, 0xe3e3e3c1c1c1f7ff, 0xc7c7c7c1c1c1f7ff, 0x8f8787c1c1c1f7ff,
    0x1f0303818181f7ff, 0x3f0101010101f7ff, 0xe0e080808080f7ff, 0xf0f080808080f7ff, 0xf1f180808080f7ff, 0xe3e380808080f7ff,
    0xc7c780808080f7ff, 0x878780808080f7ff, 0x030380808080f7ff, 0x010100000000f7ff, 0xc0c0c0c0c0c0e0e0, 0xc0c0c0c0c0c0e0e0,
    0xc0c0c0c0c0c0e0f1, 0xc1c1c1c1c1c1e3e3, 0x81818181818183c7, 0x8181818181818383, 0x0101010101010303, 0x0000000000000023,
    0xffc0c0c0c0c0e0e0, 0xffe0e0e0e0e0e0e0, 0xffe1e1e1e1e1f1f1, 0x0000000000000000, 0xffc3c3c3c3c3c7c7, 0xff83838383838383,
    0xff01010101010303, 0x8000000000000023, 0xffc1c0c1c0c0e0e3, 0xffffe0e2e0e0e0e3, 0xfffff1f5f1f1f1ff, 0xffffe3ebe3e3e3ff,
    0xffffc7d7c7c7c7ff, 0xffff83a3838383e3, 0xffc10141010103e3, 0x80800080000000e3, 0xffc1c1c0c0c0e3e3, 0xffffe3e0e0e0e3e3,
    0xfffffff1f1e1e3e3, 0xffffffe3e3e3e3e3, 0xffffffc7c7c3e3e3, 0xffffe3838383e3e3, 0xffc1c1010101e3e3, 0x80808000000080e3,
    0xffc1c0c0c0c1e3e3, 0xffffe0e0e0e3e3e3, 0xfffff1f1e1e3e3e3, 0xffffe3e3e3e3e3e3, 0xffffc7c7c3e3e3e3, 0xffff838383e3e3e3,
    0xffc1010101c1e3e3, 0x80800000008080e3, 0xffc0c0c0c0c0e3e3, 0xffe0e0c0c0c0e3e3, 0xfff1f1c1c1c1e3e3, 0xffe3e3c1c1c1e3e3,
    0xffc7c7c1c1c1e3e3, 0xff8383818181e3e3, 0xff0101010101e3e3, 0x80000000000080e3, 0xc0c08080808080e3, 0xe0e08080808080e3,
    0xf1f18080808080e3, 0xe3e38080808080e3, 0xc7c78080808080e3, 0x83838080808080e3, 0x01010000000000e3, 0x00000000000000e3,
    0xc0000000000000e3, 0xe0000000000000e3, 0xf1000000000000e3, 0xe3000000000000e3, 0xc7000000000000e3, 0x83000000000000e3,
    0x01000000000000e3, 0x00000000000000e3, 0xc0c0c0c0c0c0fcfc, 0xc0c0c0c0c0c0f8f8, 0xc0c0c0c0c0c0f1f1, 0xc0c0c0c0c0c0e3e3,
    0xc0c0c0c0c0c0c7c7, 0xc0c0c0c0c0c08f8f, 0xc0c0c0c0c0c01f1f, 0xc0c0c0c0c0c03f3f, 0xc0c0c0c0c0f8fcfc, 0xc0c0c0c0c0f8f8f8,
    0xc0c0c0c0c0f0f1f1, 0xc0c0c0c0c0e0e3e3, 0xc0c0c0c0c0c0c7c7, 0xc0c0c0c0c0808f8f, 0xc0c0c0c0c0001f1f, 0xc0c0c0c0c0003f3f,
    0x0000000000000000, 0xc0c0c0c0f8f8f8ff, 0xc0c0c0c0f0f0f1ff, 0xc0c0c0c0e0e0e3ff, 0xc0c0c0c0c0c0c7ff, 0xc0c0c0c080808fff,
    0xc0c0c0c000001fff, 0xc0c0c0c000003fff, 0xe0e0e0f8f8fcffff, 0xe0e0e0f8f8f8ffff, 0xe0e0e0f0f0f0ffff, 0xe0e0e0e0e0e0ffff,
    0xe0e0e0c0c0c0ffff, 0xc0c0c0808080ffff, 0xc0c0c0000000ffff, 0xc0c0c0000000ffff, 0xf0f0f8f8f8feffff, 0xf0f0f8f8f8feffff,
    0xf0f0f0f0f0fcffff, 0xf0f0e0e0e0f8ffff, 0xe0e0c0c0c0f0ffff, 0xc0c0808080e0ffff, 0xc0c0000000c0ffff, 0xc0c0000000c0ffff,
    0xf8f8f8f8f8fcffff, 0xf8f8f8f8fcfcffff, 0xf8f0f0f0f8fcffff, 0xf0e0e0e0f0f8ffff, 0xe0c0c0c0e0f0ffff, 0xc0808080c0e0ffff,
    0xc0000000c0c0ffff, 0xc0000000c0c0ffff, 0xf8f8f8f8f8f8ffff, 0xf8f8f8f8f8f8ffff, 0xf0f0f0f8f8f8ffff, 0xe0e0e0f0f0f0ffff,
    0xc0c0c0e0e0e0ffff, 0x808080c0c0c0ffff, 0x000000c0c0c0ffff, 0x000000c0c0c0ffff, 0xf8f8f0f0f0f0ffff, 0xf8f8f0f0f0f0ffff,
    0xf0f0f0f0f0f0ffff, 0xe0e0f0f0f0f0ffff, 0xc0c0e0e0e0e0ffff, 0x8080c0c0c0c0ffff, 0x0000c0c0c0c0ffff, 0x0000c0c0c0c0ffff,
    0x80808080808080fc, 0x80808080808080f8, 0x80808080808080f1, 0x80808080808080e3, 0x80808080808080c7, 0x808080808080808f,
    0x808080808080001f, 0x808080808080003f, 0x808080808080f0fc, 0x808080808080f8f8, 0x808080808080f0f1, 0x808080808080e0e3,
    0x808080808080c0c7, 0x808080808080808f, 0x808080808000001f, 0x808080808000003f, 0x0000000000000000, 0x8080808080f8f8ff,
    0x8080808080f0f0ff, 0x8080808080e0e0ff, 0x8080808080c0c0ff, 0x80808080808080ff, 0x80808080000000ff, 0x80808080000000ff,
    0xc0c0c0c0f0f0ffff, 0xc0c0c0c0f8f8ffff, 0xc0c0c0c0f0f0fcff, 0xc0c0c0c0e0e0f8ff, 0xc0c0c0c0c0c0f0ff, 0x808080808080e0ff,
    0x808080000000c0ff, 0x80808000000080ff, 0xe0e0e0f0f0f0fcff, 0xe0e0e0f8f8fcfcff, 0xe0e0e0f0f0f8f8ff, 0xe0e0e0e0e0f0f0ff,
    0xc0c0c0c0c0e0e0ff, 0x8080808080c0c0ff, 0x80800000008080ff, 0x80800000008080ff, 0xf0f0f0f0f0f0f8ff, 0xf0f0f8f8f8f8f8ff,
    0xf0f0f0f0f0f0f8ff, 0xe0e0e0e0e0e0f0ff, 0xc0c0c0c0c0c0e0ff, 0x808080808080c0ff, 0x80000000808080ff, 0x80000000808080ff,
    0xf0f0f0f0f0f0f0ff, 0xf8f8f0f0f0f0f0ff, 0xf0f0f0f0f0f0f0ff, 0xe0e0e0e0e0e0e0ff, 0xc0c0c0c0c0c0c0ff, 0x80808080808080ff,
    0x00000080808080ff, 0x00000080808080ff, 0xf0e0e0e0e0e0e0ff, 0xf8e0e0e0e0e0e0ff, 0xf0e0e0e0e0e0e0ff, 0xe0e0e0e0e0e0e0ff,
    0xc0c0c0c0c0c0c0ff, 0x80808080808080ff, 0x00008080808080ff, 0x00008080808080ff, 0xf0f0f0f0f0f8fcfc, 0xf0f0f0f0f0f8f8f8,
    0xf0f0f0f0f0f8f1f1, 0xe0e0e0e0e0e0e3e3, 0xe0e0e0e0e0e0c7c7, 0xc0c0c0c0c0c08f8f, 0x8080808080801f1f, 0x8080808080803f3f,
    0xf8f8f8f8f8fcfcfc, 0xf8f8f8f8f8f8f8f8, 0xf0f0f0f0f0f1f1f1, 0xf0f0f0f0f0e1e3e3, 0xe0e0e0e0e0c0c7c7, 0xc0c0c0c0c0808f8f,
    0x8080808080001f1f, 0x8080808080003f3f, 0xfffcfffcf8fcfcff, 0x0000000000000000, 0xfff9fff9f0f1f1ff, 0xfff1fdf1e0e1e3ff,
    0xffe0f8e0c0c0c7ff, 0xf0c0f0c080808fff, 0xe080e08000001fff, 0xc080c08000003fff, 0xfffffefcf8fcffff, 0xfffffdf8f8f8ffff,
    0xfffffbf1f0f1ffff, 0xfffff5e1e0e1ffff, 0xfff8e8c0c0c0ffff, 0xf0f0d0808080ffff, 0xe0e0a0000000ffff, 0xc0c0c0000000ffff,
    0xfffffcfcf8fdffff, 0xfffff8f8f8fdffff, 0xfffff1f1f0fdffff, 0xffffe1e1e0f9ffff, 0xfff8c0c0c0f0ffff, 0xf0f0808080e0ffff,
    0xe0e0000000c0ffff, 0xc0c000000080ffff, 0xfffcfcfcf8f8ffff, 0xfff8f8f8f8f8ffff, 0xfff1f1f1f8f8ffff, 0xffe3e1e1f0f8ffff,
    0xffc0c0c0e0f0ffff, 0xf0808080c0e0ffff, 0xe000000080c0ffff, 0xc00000008080ffff, 0xfcfcfcf8f0f0ffff, 0xf8f8f8f8f0f0ffff,
    0xf1f1f1f8f0f0ffff, 0xe3e1e1f0f0f0ffff, 0xc7c0c0e0e0e0ffff, 0x808080c0c0c0ffff, 0x000000808080ffff, 0x000000808080ffff,
    0xfcfcf0e0e0e0ffff, 0xf8f8f0e0e0e0ffff, 0xf1f1f0e0e0e0ffff, 0xe1e1f0e0e0e0ffff, 0xc0c0e0e0e0e0ffff, 0x8080c0c0c0c0ffff,
    0x000080808080ffff, 0x000080808080ffff, 0xe0e0e0e0e0e0f0fc, 0xe0e0e0e0e0e0f0f8, 0xe0e0e0e0e0e0f0f1, 0xc0c0c0c0c0c0c0e3,
    0xc0c0c0c0c0c0c0c7, 0x808080808080808f, 0x000000000000001f, 0x000000000000003f, 0xf0f0f0f0f0f8fcfc, 0xf0f0f0f0f0f8f8f8,
    0xe0e0e0e0e0e0f1f1, 0xe0e0e0e0e0e0e0e3, 0xc0c0c0c0c0c0c0c7, 0x808080808080808f, 0x000000000000001f, 0x000000000000003f,
    0xf8f8f8f8f8fcfcff, 0x0000000000000000, 0xf0f0f0f0f0f1f1ff, 0xe0e0e0e0e0e0e0ff, 0xc0c0c0c0c0c0c0ff, 0x80808080808080ff,
    0x00000000000000ff, 0x00000000000000ff, 0xfffcfdfcfcfcffff, 0xfff8faf8f8f8ffff, 0xfff1f5f1f1f1ffff, 0xffe0e8e0e0e0f8ff,
    0xf0c0d0c0c0c0f0ff, 0xe080a0808080e0ff, 0xc00040000000c0ff, 0x80808000000080ff, 0xfffffcfcf8f8f8ff, 0xfffff8f8f8f8f8ff,
    0xfffff1f1f0f8f8ff, 0xfff8e0e0e0f0f0ff, 0xf0f0c0c0c0e0e0ff, 0xe0e0808080c0c0ff, 0xc0c00000008080ff, 0x80800000000000ff,
    0xfffcfcf8f0f0f0ff, 0xfff8f8f8f0f0f0ff, 0xfff1f1f0f0f0f0ff, 0xffe0e0e0e0e0f0ff, 0xf0c0c0c0c0c0e0ff, 0xe08080808080c0ff,
    0xc0000000000080ff, 0x80000000000000ff, 0xfcfcf0e0e0e0e0ff, 0xf8f8f0e0e0e0e0ff, 0xf1f1f0e0e0e0e0ff, 0xe0e0e0e0e0e0e0ff,
    0xc0c0c0c0c0c0c0ff, 0x80808080808080ff, 0x00000000000000ff, 0x00000000000000ff, 0xfce0c0c0c0c0c0ff, 0xf8e0c0c0c0c0c0ff,
    0xf1e0c0c0c0c0c0ff, 0xe0e0c0c0c0c0c0ff, 0xc0c0c0c0c0c0c0ff, 0x80808080808080ff, 0x00000000000000ff, 0x00000000000000ff,
    0xe0e0e0e0e0f0fcfc, 0xe0e0e0e0e0f1f8f8, 0xe0e0e0e0e0f1f1f1, 0xe0e0e0e0e0f1e3e3, 0xc0c0c0c0c0c1c7c7, 0xc0c0c0c0c0c18f8f,
    0x8080808080801f1f, 0x0000000000003f3f, 0xf0f0f0f0f0f8fcfc, 0xf0f0f0f0f0f8f8f8, 0xf1f1f1f1f1f1f1f1, 0xe1e1e1e1e1e3e3e3,
    0xe1e1e1e1e1c3c7c7, 0xc1c1c1c1c1818f8f, 0x8080808080001f1f, 0x0000000000003f3f, 0xfff8fbf8f0f8fcff, 0xfff9fff9f0f8f8ff,
    0x0000000000000000, 0xfff3fff3e1e3e3ff, 0xffe3fbe3c1c3c7ff, 0xffc1f1c181818fff, 0xe080e08000001fff, 0xc000c00000003fff,
    0xfffffaf8f0f8ffff, 0xfffffdf8f0f8ffff, 0xfffffbf1f1f1ffff, 0xfffff7e3e1e3ffff, 0xffffebc3c1c3ffff, 0xfff1d1818181ffff,
    0xe0e0a0000000ffff, 0xc0c040000000ffff, 0xfffff8f8f0f9ffff, 0xfffff8f8f0fbffff, 0xfffff1f1f1fbffff, 0xffffe3e3e1fbffff,
    0xffffc3c3c1f3ffff, 0xfff1818181e1ffff, 0xe0e0000000c0ffff, 0xc0c000000080ffff, 0xfffcf8f8f0f1ffff, 0xfff8f8f8f1f1ffff,
    0xfff1f1f1f1f1ffff, 0xffe3e3e3f1f1ffff, 0xffc7c3c3e1f1ffff, 0xff818181c1e1ffff, 0xe000000080c0ffff, 0xc00000000080ffff,
    0xfcf8f8f0e0e0ffff, 0xf8f8f8f1e0e0ffff, 0xf1f1f1f1e0e0ffff, 0xe3e3e3f1e0e0ffff, 0xc7c3c3e1e0e0ffff, 0x8f8181c1c0c0ffff,
    0x000000808080ffff, 0x000000000000ffff, 0xf8f8e0c0c0c0ffff, 0xf8f8e0c0c0c0ffff, 0xf1f1e0c0c0c0ffff, 0xe3e3e0c0c0c0ffff,
    0xc3c3e0c0c0c0ffff, 0x8181c0c0c0c0ffff, 0x000080808080ffff, 0x000000000000ffff, 0xc0c0c0c0c0c0e0fc, 0xc0c0c0c0c0c0e0f8,
    0xc0c0c0c0c0c0e0f1, 0xc0c0c0c0c0c0e0e3, 0x80808080808080c7, 0x808080808080808f, 0x000000000000001f, 0x000000000000003f,
    0xe0e0e0e0e0f0f0fc, 0xe0e0e0e0e0f0f8f8, 0xe0e0e0e0e0f1f1f1, 0xc0c0c0c0c0c1e3e3, 0xc0c0c0c0c0c1c1c7, 0x808080808080808f,
    0x000000000000001f, 0x000000000000003f, 0xf0f0f0f0f0f0f0ff, 0xf0f0f0f0f0f8f8ff, 0x0000000000000000, 0xe1e1e1e1e1e3e3ff,
    0xc1c1c1c1c1c1c1ff, 0x80808080808080ff, 0x00000000000000ff, 0x00000000000000ff, 0xfff0f1f0f0f0f1ff, 0xfff8faf8f8f8ffff,
    0xfff1f5f1f1f1ffff, 0xffe3ebe3e3e3ffff, 0xffc1d1c1c1c1f1ff, 0xe080a0808080e0ff, 0xc00040000000c0ff, 0x80008000000080ff,
    0xfff1f0f0f0f0f0ff, 0xfffff8f8f0f1f1ff, 0xfffff1f1f1f1f1ff, 0xffffe3e3e1f1f1ff, 0xfff1c1c1c1e1e1ff, 0xe0e0808080c0c0ff,
    0xc0c00000008080ff, 0x80800000000000ff, 0xfff0f0f0e0e0e0ff, 0xfff8f8f0e0e0e0ff, 0xfff1f1f1e0e0e0ff, 0xffe3e3e1e0e0e0ff,
    0xffc1c1c1c0c0e0ff, 0xe08080808080c0ff, 0xc0000000000080ff, 0x80000000000000ff, 0xf0f0e0c0c0c0c0ff, 0xf8f8e0c0c0c0c0ff,
    0xf1f1e0c0c0c0c0ff, 0xe3e3e0c0c0c0c0ff, 0xc1c1c0c0c0c0c0ff, 0x80808080808080ff, 0x00000000000000ff, 0x00000000000000ff,
    0xf0c08080808080ff, 0xf8c08080808080ff, 0xf1c08080808080ff, 0xe3c08080808080ff, 0xc1c08080808080ff, 0x80808080808080ff,
    0x00000000000000ff, 0x00000000000000ff, 0xc0c0c0c0c0e0fcfc, 0xc0c0c0c0c0e0f8f8, 0xc1c1c1c1c1e3f1f1, 0xc1c1c1c1c1e3e3e3,
    0xc1c1c1c1c1e3c7c7, 0x8181818181838f8f, 0x8181818181831f1f, 0x0101010101013f3f, 0xe0e0e0e0e0e0fcfc, 0xe1e1e1e1e1f0f8f8,
    0xe1e1e1e1e1f1f1f1, 0xe3e3e3e3e3e3e3e3, 0xc3c3c3c3c3c7c7c7, 0xc3c3c3c3c3878f8f, 0x8383838383031f1f, 0x0101010101013f3f,
    0xffe0e3e0e0e0fcff, 0xfff1f7f1e0f0f8ff, 0xfff3fff3e1f1f1ff, 0x0000000000000000, 0xffe7ffe7c3c7c7ff, 0xffc7f7c783878fff,
    0xff83e38303031fff, 0xc101c10101013fff, 0xffe3e2e0e0e0ffff, 0xfffff5f0e0f0ffff, 0xfffffbf1e1f1ffff, 0xfffff7e3e3e3ffff,
    0xffffefc7c3c7ffff, 0xffffd7878387ffff, 0xffe3a3030303ffff, 0xc1c141010101ffff, 0xffe3e0e0e0e1ffff, 0xfffff0f0e0f3ffff,
    0xfffff1f1e1f7ffff, 0xffffe3e3e3f7ffff, 0xffffc7c7c3f7ffff, 0xffff878783e7ffff, 0xffe3030303c3ffff, 0xc1c101010181ffff,
    0xffe0e0e0e0e1ffff, 0xfff8f0f0e1e3ffff, 0xfff1f1f1e3e3ffff, 0xffe3e3e3e3e3ffff, 0xffc7c7c7e3e3ffff, 0xff8f8787c3e3ffff,
    0xff03030383c3ffff, 0xc10101010181ffff, 0xfce0e0e0c0c0ffff, 0xf8f0f0e1c1c1ffff, 0xf1f1f1e3c1c1ffff, 0xe3e3e3e3c1c1ffff,
    0xc7c7c7e3c1c1ffff, 0x8f8787c3c1c1ffff, 0x1f0303838181ffff, 0x010101010101ffff, 0xe0e0c0808080ffff, 0xf0f0c1808080ffff,
    0xf1f1c1808080ffff, 0xe3e3c1808080ffff, 0xc7c7c1808080ffff, 0x8787c1808080ffff, 0x030381808080ffff, 0x010101000000ffff,
    0x808080808080c0fc, 0x808080808080c0f8, 0x808080808080c1f1, 0x808080808080c1e3, 0x808080808080c1c7, 0x000000000000018f,
    0x000000000000011f, 0x000000000000003f, 0xc0c0c0c0c0c0c0fc, 0xc0c0c0c0c0e0e0f8, 0xc0c0c0c0c0e0f1f1, 0xc1c1c1c1c1e3e3e3,
    0x818181818183c7c7, 0x818181818183838f, 0x010101010101011f, 0x000000000000003f, 0xc0c0c0c0c0c0c0ff, 0xe0e0e0e0e0e0e0ff,
    0xe1e1e1e1e1f1f1ff, 0x0000000000000000, 0xc3c3c3c3c3c7c7ff, 0x83838383838383ff, 0x01010101010101ff, 0x00000000000000ff,
    0xc1c0c1c0c0c0c1ff, 0xffe0e2e0e0e0e3ff, 0xfff1f5f1f1f1ffff, 0xffe3ebe3e3e3ffff, 0xffc7d7c7c7c7ffff, 0xff83a3838383e3ff,
    0xc10141010101c1ff, 0x80008000000080ff, 0xc1c1c0c0c0c0c0ff, 0xffe3e0e0e0e1e1ff, 0xfffff1f1e1e3e3ff, 0xffffe3e3e3e3e3ff,
    0xffffc7c7c3e3e3ff, 0xffe3838383c3c3ff, 0xc1c10101018181ff, 0x80800000000000ff, 0xc1c0c0c0c0c0c0ff, 0xffe0e0e0c0c0c1ff,
    0xfff1f1e1c1c1c1ff, 0xffe3e3e3c1c1c1ff, 0xffc7c7c3c1c1c1ff, 0xff8383838181c1ff, 0xc1010101010181ff, 0x80000000000000ff,
    0xc0c0c080808080ff, 0xe0e0c080808080ff, 0xf1f1c180808080ff, 0xe3e3c180808080ff, 0xc7c7c180808080ff, 0x83838180808080ff,
    0x01010100000000ff, 0x00000000000000ff, 0xc0800000000000ff, 0xe0800000000000ff, 0xf1800000000000ff, 0xe3800000000000ff,
    0xc7800000000000ff, 0x83800000000000ff, 0x01000000000000ff, 0x00000000000000ff, 0xe0e0e0e0e0fffcfc, 0xe0e0e0e0e0fff8f8,
    0xe0e0e0e0e0fff1f1, 0xe0e0e0e0e0ffe3e3, 0xe0e0e0e0e0ffc7c7, 0xe0e0e0e0e0ff8f8f, 0xe0e0e0e0e0ff1f1f, 0xe0e0e0e0e0ff3f3f,
    0xe0e0e0e0e0fcfcfc, 0xe0e0e0e0e0f8f8f8, 0xe0e0e0e0e0f1f1f1, 0xe0e0e0e0e0e3e3e3, 0xe0e0e0e0e0c7c7c7, 0xe0e0e0e0e08f8f8f,
    0xe0e0e0e0e01f1f1f, 0xe0e0e0e0e03f3f3f, 0xe0e0e0e0f8fcfcff, 0xe0e0e0e0f8f8f8ff, 0xe0e0e0e0f0f1f1ff, 0xe0e0e0e0e0e3e3ff,
    0xe0e0e0e0c0c7c7ff, 0xe0e0e0e0808f8fff, 0xe0e0e0e0001f1fff, 0xe0e0e0e0203f3fff, 0x0000000000000000, 0xe0e0e0f8f8f8ffff,
    0xe0e0e0f0f0f1ffff, 0xe0e0e0e0e0e3ffff, 0xe0e0e0c0c0c7ffff, 0xe0e0e080808fffff, 0xe0e0e000001fffff, 0xe0e0e020203fffff,
    0xf0f0f8f8fcffffff, 0xf0f0f8f8f8ffffff, 0xf0f0f0f0f0ffffff, 0xf0f0e0e0e0ffffff, 0xe0e0c0c0c0ffffff, 0xe0e0808080ffffff,
    0xe0e0000000ffffff, 0xe0e0202020ffffff, 0xf8f8f8f8feffffff, 0xf8f8f8f8feffffff, 0xf8f0f0f0fcffffff, 0xf0e0e0e0f8ffffff,
    0xe0c0c0c0f0ffffff, 0xe0808080e0ffffff, 0xe0000000e0ffffff, 0xe0202020e0ffffff, 0xf8f8f8f8fcffffff, 0xf8f8f8fcfcffffff,
    0xf0f0f0f8fcffffff, 0xe0e0e0f0f8ffffff, 0xc0c0c0e0f0ffffff, 0x808080e0e0ffffff, 0x000000e0e0ffffff, 0x202020e0e0ffffff,
    0xf8f8f8f8f8ffffff, 0xf8f8f8f8f8ffffff, 0xf0f0f8f8f8ffffff, 0xe0e0f0f0f0ffffff, 0xc0c0e0e0e0ffffff, 0x8080e0e0e0ffffff,
    0x0000e0e0e0ffffff, 0x2020e0e0e0ffffff, 0xc0c0c0c0c0c0fcfc, 0xc0c0c0c0c0c0f8f8, 0xc0c0c0c0c0c0f1f1, 0xc0c0c0c0c0c0e3e3,
    0xc0c0c0c0c0c0c7c7, 0xc0c0c0c0c0c08f8f, 0xc0c0c0c0c0c01f1f, 0xc0c0c0c0c0c03f3f, 0xc0c0c0c0c0c0fcfc, 0xc0c0c0c0c0c0f8f8,
    0xc0c0c0c0c0c0f1f1, 0xc0c0c0c0c0c0e3e3, 0xc0c0c0c0c0c0c7c7, 0xc0c0c0c0c0808f8f, 0xc0c0c0c0c0001f1f, 0xc0c0c0c0c0003f3f,
    0xc0c0c0c0c0f0fcff, 0xc0c0c0c0c0f8f8ff, 0xc0c0c0c0c0f0f1ff, 0xc0c0c0c0c0e0e3ff, 0xc0c0c0c0c0c0c7ff, 0xc0c0c0c080808fff,
    0xc0c0c0c000001fff, 0xc0c0c0c000003fff, 0x0000000000000000, 0xc0c0c0c0f8f8ffff, 0xc0c0c0c0f0f0ffff, 0xc0c0c0c0e0e0ffff,
    0xc0c0c0c0c0c0ffff, 0xc0c0c0808080ffff, 0xc0c0c0000000ffff, 0xc0c0c0000000ffff, 0xe0e0e0f0f0ffffff, 0xe0e0e0f8f8ffffff,
    0xe0e0e0f0f0fcffff, 0xe0e0e0e0e0f8ffff, 0xc0c0c0c0c0f0ffff, 0xc0c0808080e0ffff, 0xc0c0000000c0ffff, 0xc0c0000000c0ffff,
    0xf0f0f0f0f0fcffff, 0xf0f0f8f8fcfcffff, 0xf0f0f0f0f8f8ffff, 0xe0e0e0e0f0f0ffff, 0xc0c0c0c0e0e0ffff, 0xc0808080c0c0ffff,
    0xc0000000c0c0ffff, 0xc0000000c0c0ffff, 0xf0f0f0f0f0f8ffff, 0xf8f8f8f8f8f8ffff, 0xf0f0f0f0f0f8ffff, 0xe0e0e0e0e0f0ffff,
    0xc0c0c0c0c0e0ffff, 0x808080c0c0c0ffff, 0x000000c0c0c0ffff, 0x000000c0c0c0ffff, 0xf0f0f0f0f0f0ffff, 0xf8f0f0f0f0f0ffff,
    0xf0f0f0f0f0f0ffff, 0xe0e0e0e0e0e0ffff, 0xc0c0c0c0c0c0ffff, 0x8080c0c0c0c0ffff, 0x0000c0c0c0c0ffff, 0x0000c0c0c0c0ffff,
    0xe0e0e0e0e0fffcfc, 0xe0e0e0e0e0fff8f8, 0xe0e0e0e0e0fff1f1, 0xe0e0e0e0e0ffe3e3, 0xc0c0c0c0c0ffc7c7, 0xc0c0c0c0c0ff8f8f,
    0xc0c0c0c0c0ff1f1f, 0xc0c0c0c0c0ff3f3f, 0xf0f0f0f0f8fcfcfc, 0xf0f0f0f0f8f8f8f8, 0xf0f0f0f0f8f1f1f1, 0xe0e0e0e0e0e3e3e3,
    0xe0e0e0e0e0c7c7c7, 0xc0c0c0c0c08f8f8f, 0xc0c0c0c0c01f1f1f, 0xc0c0c0c0c03f3f3f, 0xf8f8f8f8fcfcfcff, 0xf8f8f8f8f8f8f8ff,
    0xf0f0f0f0f1f1f1ff, 0xf0f0f0f0e1e3e3ff, 0xe0e0e0e0c0c7c7ff, 0xc0c0c0c0808f8fff, 0xc0c0c0c0001f1fff, 0xc0c0c0c0003f3fff,
    0xfcfffcf8fcfcffff, 0x0000000000000000, 0xf9fff9f0f1f1ffff, 0xf1fdf1e0e1e3ffff, 0xe0f8e0c0c0c7ffff, 0xc0f0c080808fffff,
    0xc0e0c000001fffff, 0xc0c0c000003fffff, 0xfffefcf8fcffffff, 0xfffdf8f8f8ffffff, 0xfffbf1f0f1ffffff, 0xfff5e1e0e1ffffff,
    0xf8e8c0c0c0ffffff, 0xf0d0808080ffffff, 0xe0e0000000ffffff, 0xc0c0000000ffffff, 0xfffcfcf8fdffffff, 0xfff8f8f8fdffffff,
    0xfff1f1f0fdffffff, 0xffe1e1e0f9ffffff, 0xf8c0c0c0f0ffffff, 0xf0808080e0ffffff, 0xe0000000c0ffffff, 0xc0000000c0ffffff,
    0xfcfcfcf8f8ffffff, 0xf8f8f8f8f8ffffff, 0xf1f1f1f8f8ffffff, 0xe3e1e1f0f8ffffff, 0xc0c0c0e0f0ffffff, 0x808080c0e0ffffff,
    0x000000c0c0ffffff, 0x000000c0c0ffffff, 0xfcfcf8f0f0ffffff, 0xf8f8f8f0f0ffffff, 0xf1f1f8f0f0ffffff, 0xe1e1f0f0f0ffffff,
    0xc0c0e0e0e0ffffff, 0x8080c0c0c0ffffff, 0x0000c0c0c0ffffff, 0x0000c0c0c0ffffff, 0xc0c0c0c0c0c0fcfc, 0xc0c0c0c0c0c0f8f8,
    0xc0c0c0c0c0c0f1f1, 0xc0c0c0c0c0c0e3e3, 0x808080808080c7c7, 0x8080808080808f8f, 0x8080808080801f1f, 0x8080808080803f3f,
    0xe0e0e0e0e0f0fcfc, 0xe0e0e0e0e0f0f8f8, 0xe0e0e0e0e0f0f1f1, 0xc0c0c0c0c0c0e3e3, 0xc0c0c0c0c0c0c7c7, 0x8080808080808f8f,
    0x8080808080001f1f, 0x8080808080003f3f, 0xf0f0f0f0f8fcfcff, 0xf0f0f0f0f8f8f8ff, 0xe0e0e0e0e0f1f1ff, 0xe0e0e0e0e0e0e3ff,
    0xc0c0c0c0c0c0c7ff, 0x8080808080808fff, 0x8080808000001fff, 0x8080808000003fff, 0xf8f8f8f8fcfcffff, 0x0000000000000000,
    0xf0f0f0f0f1f1ffff, 0xe0e0e0e0e0e0ffff, 0xc0c0c0c0c0c0ffff, 0x808080808080ffff, 0x808080000000ffff, 0x808080000000ffff,
    0xfcfdfcfcfcffffff, 0xf8faf8f8f8ffffff, 0xf1f5f1f1f1ffffff, 0xe0e8e0e0e0f8ffff, 0xc0d0c0c0c0f0ffff, 0x80a0808080e0ffff,
    0xc0c0000000c0ffff, 0x808000000080ffff, 0xfffcfcf8f8f8ffff, 0xfff8f8f8f8f8ffff, 0xfff1f1f0f8f8ffff, 0xf8e0e0e0f0f0ffff,
    0xf0c0c0c0e0e0ffff, 0xe0808080c0c0ffff, 0xc00000008080ffff, 0x800000008080ffff, 0xfcfcf8f0f0f0ffff, 0xf8f8f8f0f0f0ffff,
    0xf1f1f0f0f0f0ffff, 0xe0e0e0e0e0f0ffff, 0xc0c0c0c0c0e0ffff, 0x8080808080c0ffff, 0x000000808080ffff, 0x000000808080ffff,
    0xfcf0e0e0e0e0ffff, 0xf8f0e0e0e0e0ffff, 0xf1f0e0e0e0e0ffff, 0xe0e0e0e0e0e0ffff, 0xc0c0c0c0c0c0ffff, 0x808080808080ffff,
    0x000080808080ffff, 0x000080808080ffff, 0xc0c0c0c0c0fffcfc, 0xc0c0c0c0c0fff8f8, 0xc0c0c0c0c0fff1f1, 0xc0c0c0c0c0ffe3e3,
    0xc0c0c0c0c0ffc7c7, 0x8080808080ff8f8f, 0x8080808080ff1f1f, 0x8080808080ff3f3f, 0xe0e0e0e0f0fcfcfc, 0xe0e0e0e0f1f8f8f8,
    0xe0e0e0e0f1f1f1f1, 0xe0e0e0e0f1e3e3e3, 0xc0c0c0c0c1c7c7c7, 0xc0c0c0c0c18f8f8f, 0x80808080801f1f1f, 0x80808080803f3f3f,
    0xf0f0f0f0f8fcfcff, 0xf0f0f0f0f8f8f8ff, 0xf1f1f1f1f1f1f1ff, 0xe1e1e1e1e3e3e3ff, 0xe1e1e1e1c3c7c7ff, 0xc1c1c1c1818f8fff,
    0x80808080001f1fff, 0x80808080003f3fff, 0xf8fbf8f0f8fcffff, 0xf9fff9f0f8f8ffff, 0x0000000000000000, 0xf3fff3e1e3e3ffff,
    0xe3fbe3c1c3c7ffff, 0xc1f1c181818fffff, 0x80e08000001fffff, 0x80c08000003fffff, 0xfffaf8f0f8ffffff, 0xfffdf8f0f8ffffff,
    0xfffbf1f1f1ffffff, 0xfff7e3e1e3ffffff, 0xffebc3c1c3ffffff, 0xf1d1818181ffffff, 0xe0a0000000ffffff, 0xc0c0000000ffffff,
    0xfff8f8f0f9ffffff, 0xfff8f8f0fbffffff, 0xfff1f1f1fbffffff, 0xffe3e3e1fbffffff, 0xffc3c3c1f3ffffff, 0xf1818181e1ffffff,
    0xe0000000c0ffffff, 0xc000000080ffffff, 0xfcf8f8f0f1ffffff, 0xf8f8f8f1f1ffffff, 0xf1f1f1f1f1ffffff, 0xe3e3e3f1f1ffffff,
    0xc7c3c3e1f1ffffff, 0x818181c1e1ffffff, 0x00000080c0ffffff, 0x0000008080ffffff, 0xf8f8f0e0e0ffffff, 0xf8f8f1e0e0ffffff,
    0xf1f1f1e0e0ffffff, 0xe3e3f1e0e0ffffff, 0xc3c3e1e0e0ffffff, 0x8181c1c0c0ffffff, 0x0000808080ffffff, 0x0000808080ffffff,
    0x808080808080fcfc, 0x808080808080f8f8, 0x808080808080f1f1, 0x808080808080e3e3, 0x808080808080c7c7, 0x0000000000008f8f,
    0x0000000000001f1f, 0x0000000000003f3f, 0xc0c0c0c0c0e0fcfc, 0xc0c0c0c0c0e0f8f8, 0xc0c0c0c0c0e0f1f1, 0xc0c0c0c0c0e0e3e3,
    0x808080808080c7c7, 0x8080808080808f8f, 0x0000000000001f1f, 0x0000000000003f3f, 0xe0e0e0e0f0f0fcff, 0xe0e0e0e0f0f8f8ff,
    0xe0e0e0e0f1f1f1ff, 0xc0c0c0c0c1e3e3ff, 0xc0c0c0c0c1c1c7ff, 0x8080808080808fff, 0x0000000000001fff, 0x0000000000003fff,
    0xf0f0f0f0f0f0ffff, 0xf0f0f0f0f8f8ffff, 0x0000000000000000, 0xe1e1e1e1e3e3ffff, 0xc1c1c1c1c1c1ffff, 0x808080808080ffff,
    0x000000000000ffff, 0x000000000000ffff, 0xf0f1f0f0f0f1ffff, 0xf8faf8f8f8ffffff, 0xf1f5f1f1f1ffffff, 0xe3ebe3e3e3ffffff,
    0xc1d1c1c1c1f1ffff, 0x80a0808080e0ffff, 0x0040000000c0ffff, 0x808000000080ffff, 0xf1f0f0f0f0f0ffff, 0xfff8f8f0f1f1ffff,
    0xfff1f1f1f1f1ffff, 0xffe3e3e1f1f1ffff, 0xf1c1c1c1e1e1ffff, 0xe0808080c0c0ffff, 0xc00000008080ffff, 0x800000000000ffff,
    0xf0f0f0e0e0e0ffff, 0xf8f8f0e0e0e0ffff, 0xf1f1f1e0e0e0ffff, 0xe3e3e1e0e0e0ffff, 0xc1c1c1c0c0e0ffff, 0x8080808080c0ffff,
    0x000000000080ffff, 0x000000000000ffff, 0xf0e0c0c0c0c0ffff, 0xf8e0c0c0c0c0ffff, 0xf1e0c0c0c0c0ffff, 0xe3e0c0c0c0c0ffff,
    0xc1c0c0c0c0c0ffff, 0x808080808080ffff, 0x000000000000ffff, 0x000000000000ffff, 0x8080808080fffcfc, 0x8080808080fff8f8,
    0x8080808080fff1f1, 0x8080808080ffe3e3, 0x8080808080ffc7c7, 0x8080808080ff8f8f, 0x0000000000ff1f1f, 0x0000000000ff3f3f,
    0xc0c0c0c0e0fcfcfc, 0xc0c0c0c0e0f8f8f8, 0xc1c1c1c1e3f1f1f1, 0xc1c1c1c1e3e3e3e3, 0xc1c1c1c1e3c7c7c7, 0x81818181838f8f8f,
    0x81818181831f1f1f, 0x01010101013f3f3f, 0xe0e0e0e0e0fcfcff, 0xe1e1e1e1f0f8f8ff, 0xe1e1e1e1f1f1f1ff, 0xe3e3e3e3e3e3e3ff,
    0xc3c3c3c3c7c7c7ff, 0xc3c3c3c3878f8fff, 0x83838383031f1fff, 0x01010101013f3fff, 0xe0e3e0e0e0fcffff, 0xf1f7f1e0f0f8ffff,
    0xf3fff3e1f1f1ffff, 0x0000000000000000, 0xe7ffe7c3c7c7ffff, 0xc7f7c783878fffff, 0x83e38303031fffff, 0x01c10101013fffff,
    0xe3e2e0e0e0ffffff, 0xfff5f0e0f0ffffff, 0xfffbf1e1f1ffffff, 0xfff7e3e3e3ffffff, 0xffefc7c3c7ffffff, 0xffd7878387ffffff,
    0xe3a3030303ffffff, 0xc141010101ffffff, 0xe3e0e0e0e1ffffff, 0xfff0f0e0f3ffffff, 0xfff1f1e1f7ffffff, 0xffe3e3e3f7ffffff,
    0xffc7c7c3f7ffffff, 0xff878783e7ffffff, 0xe3030303c3ffffff, 0xc101010181ffffff, 0xe0e0e0e0e1ffffff, 0xf8f0f0e1e3ffffff,
    0xf1f1f1e3e3ffffff, 0xe3e3e3e3e3ffffff, 0xc7c7c7e3e3ffffff, 0x8f8787c3e3ffffff, 0x03030383c3ffffff, 0x0101010181ffffff,
    0xe0e0e0c0c0ffffff, 0xf0f0e1c1c1ffffff, 0xf1f1e3c1c1ffffff, 0xe3e3e3c1c1ffffff, 0xc7c7e3c1c1ffffff, 0x8787c3c1c1ffffff,
    0x0303838181ffffff, 0x0101010101ffffff, 0x000000000000fcfc, 0x000000000000f8f8, 0x000000000000f1f1, 0x000000000000e3e3,
    0x000000000000c7c7, 0x0000000000008f8f, 0x0000000000001f1f, 0x0000000000003f3f, 0x8080808080c0fcfc, 0x8080808080c0f8f8,
    0x8080808080c1f1f1, 0x8080808080c1e3e3, 0x8080808080c1c7c7, 0x0000000000018f8f, 0x0000000000011f1f, 0x0000000000003f3f,
    0xc0c0c0c0c0c0fcff, 0xc0c0c0c0e0e0f8ff, 0xc0c0c0c0e0f1f1ff, 0xc1c1c1c1e3e3e3ff, 0x8181818183c7c7ff, 0x8181818183838fff,
    0x0101010101011fff, 0x0000000000003fff, 0xc0c0c0c0c0c0ffff, 0xe0e0e0e0e0e0ffff, 0xe1e1e1e1f1f1ffff, 0x0000000000000000,
    0xc3c3c3c3c7c7ffff, 0x838383838383ffff, 0x010101010101ffff, 0x000000000000ffff, 0xc0c1c0c0c0c1ffff, 0xe0e2e0e0e0e3ffff,
    0xf1f5f1f1f1ffffff, 0xe3ebe3e3e3ffffff, 0xc7d7c7c7c7ffffff, 0x83a3838383e3ffff, 0x0141010101c1ffff, 0x008000000080ffff,
    0xc1c0c0c0c0c0ffff, 0xe3e0e0e0e1e1ffff, 0xfff1f1e1e3e3ffff, 0xffe3e3e3e3e3ffff, 0xffc7c7c3e3e3ffff, 0xe3838383c3c3ffff,
    0xc10101018181ffff, 0x800000000000ffff, 0xc0c0c0c0c0c0ffff, 0xe0e0e0c0c0c1ffff, 0xf1f1e1c1c1c1ffff, 0xe3e3e3c1c1c1ffff,
    0xc7c7c3c1c1c1ffff, 0x8383838181c1ffff, 0x010101010181ffff, 0x000000000000ffff, 0xc0c080808080ffff, 0xe0c080808080ffff,
    0xf1c180808080ffff, 0xe3c180808080ffff, 0xc7c180808080ffff, 0x838180808080ffff, 0x010100000000ffff, 0x000000000000ffff,
    0xf0f0f0f0fffffcfc, 0xf0f0f0f0fffff8f8, 0xf0f0f0f0fffff1f1, 0xf0f0f0f0ffffe3e3, 0xf0f0f0f0ffffc7c7, 0xf0f0f0f0ffff8f8f,
    0xf0f0f0f0ffff1f1f, 0xf0f0f0f0ffff3f3f, 0xf0f0f0f0fffcfcfc, 0xf0f0f0f0fff8f8f8, 0xf0f0f0f0fff1f1f1, 0xf0f0f0f0ffe3e3e3,
    0xf0f0f0f0ffc7c7c7, 0xf0f0f0f0ff8f8f8f, 0xf0f0f0f0ff1f1f1f, 0xf0f0f0f0ff3f3f3f, 0xf0f0f0f0fcfcfcff, 0xf0f0f0f0f8f8f8ff,
    0xf0f0f0f0f1f1f1ff, 0xf0f0f0f0e3e3e3ff, 0xf0f0f0f0c7c7c7ff, 0xf0f0f0f08f8f8fff, 0xf0f0f0f01f1f1fff, 0xf0f0f0f03f3f3fff,
    0xf0f0f0f8fcfcffff, 0xf0f0f0f8f8f8ffff, 0xf0f0f0f0f1f1ffff, 0xf0f0f0e0e3e3ffff, 0xf0f0f0c0c7c7ffff, 0xf0f0f0808f8fffff,
    0xf0f0f0101f1fffff, 0xf0f0f0303f3fffff, 0x0000000000000000, 0xf0f0f8f8f8ffffff, 0xf0f0f0f0f1ffffff, 0xf0f0e0e0e3ffffff,
    0xf0f0c0c0c7ffffff, 0xf0f080808fffffff, 0xf0f010101fffffff, 0xf0f030303fffffff, 0xf8f8f8fcffffffff, 0xf8f8f8f8ffffffff,
    0xf8f0f0f0ffffffff, 0xf0e0e0e0ffffffff, 0xf0c0c0c0ffffffff, 0xf0808080ffffffff, 0xf0101010ffffffff, 0xf0303030ffffffff,
    0xf8f8f8feffffffff, 0xf8f8f8feffffffff, 0xf0f0f0fcffffffff, 0xe0e0e0f8ffffffff, 0xc0c0c0f0ffffffff, 0x808080f0ffffffff,
    0x101010f0ffffffff, 0x303030f0ffffffff, 0xf8f8f8fcffffffff, 0xf8f8fcfcffffffff, 0xf0f0f8fcffffffff, 0xe0e0f0f8ffffffff,
    0xc0c0f0f0ffffffff, 0x8080f0f0ffffffff, 0x1010f0f0ffffffff, 0x3030f0f0ffffffff, 0xe0e0e0e0e0fffcfc, 0xe0e0e0e0e0fff8f8,
    0xe0e0e0e0e0fff1f1, 0xe0e0e0e0e0ffe3e3, 0xe0e0e0e0e0ffc7c7, 0xe0e0e0e0e0ff8f8f, 0xe0e0e0e0e0ff1f1f, 0xe0e0e0e0e0ff3f3f,
    0xe0e0e0e0e0fcfcfc, 0xe0e0e0e0e0f8f8f8, 0xe0e0e0e0e0f1f1f1, 0xe0e0e0e0e0e3e3e3, 0xe0e0e0e0e0c7c7c7, 0xe0e0e0e0e08f8f8f,
    0xe0e0e0e0e01f1f1f, 0xe0e0e0e0e03f3f3f, 0xe0e0e0e0e0fcfcff, 0xe0e0e0e0e0f8f8ff, 0xe0e0e0e0e0f1f1ff, 0xe0e0e0e0e0e3e3ff,
    0xe0e0e0e0c0c7c7ff, 0xe0e0e0e0808f8fff, 0xe0e0e0e0001f1fff, 0xe0e0e0e0203f3fff, 0xe0e0e0e0f0fcffff, 0xe0e0e0e0f8f8ffff,
    0xe0e0e0e0f0f1ffff, 0xe0e0e0e0e0e3ffff, 0xe0e0e0c0c0c7ffff, 0xe0e0e080808fffff, 0xe0e0e000001fffff, 0xe0e0e020203fffff,
    0x0000000000000000, 0xe0e0e0f8f8ffffff, 0xe0e0e0f0f0ffffff, 0xe0e0e0e0e0ffffff, 0xe0e0c0c0c0ffffff, 0xe0e0808080ffffff,
    0xe0e0000000ffffff, 0xe0e0202020ffffff, 0xf0f0f0f0ffffffff, 0xf0f0f8f8ffffffff, 0xf0f0f0f0fcffffff, 0xe0e0e0e0f8ffffff,
    0xe0c0c0c0f0ffffff, 0xe0808080e0ffffff, 0xe0000000e0ffffff, 0xe0202020e0ffffff, 0xf0f0f0f0fcffffff, 0xf8f8f8fcfcffffff,
    0xf0f0f0f8f8ffffff, 0xe0e0e0f0f0ffffff, 0xc0c0c0e0e0ffffff, 0x808080e0e0ffffff, 0x000000e0e0ffffff, 0x202020e0e0ffffff,
    0xf0f0f0f0f8ffffff, 0xf8f8f8f8f8ffffff, 0xf0f0f0f0f8ffffff, 0xe0e0e0e0f0ffffff, 0xc0c0e0e0e0ffffff, 0x8080e0e0e0ffffff,
    0x0000e0e0e0ffffff, 0x2020e0e0e0ffffff, 0xe0e0e0e0fffffcfc, 0xe0e0e0e0fffff8f8, 0xe0e0e0e0fffff1f1, 0xe0e0e0e0ffffe3e3,
    0xe0e0e0e0ffffc7c7, 0xe0e0e0e0ffff8f8f, 0xe0e0e0e0ffff1f1f, 0xe0e0e0e0ffff3f3f, 0xe0e0e0e0fffcfcfc, 0xe0e0e0e0fff8f8f8,
    0xe0e0e0e0fff1f1f1, 0xe0e0e0e0ffe3e3e3, 0xe0e0e0e0ffc7c7c7, 0xe0e0e0e0ff8f8f8f, 0xe0e0e0e0ff1f1f1f, 0xe0e0e0e0ff3f3f3f,
    0xf0f0f0f8fcfcfcff, 0xf0f0f0f8f8f8f8ff, 0xf0f0f0f8f1f1f1ff, 0xe0e0e0e0e3e3e3ff, 0xe0e0e0e0c7c7c7ff, 0xe0e0e0e08f8f8fff,
    0xe0e0e0e01f1f1fff, 0xe0e0e0e03f3f3fff, 0xf8f8f8fcfcfcffff, 0xf8f8f8f8f8f8ffff, 0xf0f0f0f1f1f1ffff, 0xf0f0f0e1e3e3ffff,
    0xf0f0e0c0c7c7ffff, 0xe0e0e0808f8fffff, 0xe0e0e0001f1fffff, 0xe0e0e0203f3fffff, 0xfffcf8fcfcffffff, 0x0000000000000000,
    0xfff9f0f1f1ffffff, 0xfff9e0e1e3ffffff, 0xf8f0c0c0c7ffffff, 0xf0e080808fffffff, 0xe0e000001fffffff, 0xe0e020203fffffff,
    0xfffcf8fcffffffff, 0xfff8f8f8ffffffff, 0xfff1f0f1ffffffff, 0xffe1e0e1ffffffff, 0xf8c0c0c0ffffffff, 0xf0808080ffffffff,
    0xe0000000ffffffff, 0xe0202020ffffffff, 0xfcfcf8fdffffffff, 0xf8f8f8fdffffffff, 0xf1f1f0fdffffffff, 0xe3e1e0f9ffffffff,
    0xc0c0c0f0ffffffff, 0x808080e0ffffffff, 0x000000e0ffffffff, 0x202020e0ffffffff, 0xfcfcf8f8ffffffff, 0xf8f8f8f8ffffffff,
    0xf1f1f8f8ffffffff, 0xe1e1f0f8ffffffff, 0xc0c0e0f0ffffffff, 0x8080e0e0ffffffff, 0x0000e0e0ffffffff, 0x2020e0e0ffffffff,
    0xc0c0c0c0c0fffcfc, 0xc0c0c0c0c0fff8f8, 0xc0c0c0c0c0fff1f1, 0xc0c0c0c0c0ffe3e3, 0xc0c0c0c0c0ffc7c7, 0xc0c0c0c0c0ff8f8f,
    0xc0c0c0c0c0ff1f1f, 0xc0c0c0c0c0ff3f3f, 0xc0c0c0c0c0fcfcfc, 0xc0c0c0c0c0f8f8f8, 0xc0c0c0c0c0f1f1f1, 0xc0c0c0c0c0e3e3e3,
    0xc0c0c0c0c0c7c7c7, 0xc0c0c0c0c08f8f8f, 0xc0c0c0c0c01f1f1f, 0xc0c0c0c0c03f3f3f, 0xe0e0e0e0f0fcfcff, 0xe0e0e0e0f0f8f8ff,
    0xe0e0e0e0f0f1f1ff, 0xc0c0c0c0c0e3e3ff, 0xc0c0c0c0c0c7c7ff, 0xc0c0c0c0808f8fff, 0xc0c0c0c0001f1fff, 0xc0c0c0c0003f3fff,
    0xf0f0f0f8fcfcffff, 0xf0f0f0f8f8f8ffff, 0xe0e0e0e0f1f1ffff, 0xe0e0e0e0e0e3ffff, 0xe0c0c0c0c0c7ffff, 0xc0c0c080808fffff,
    0xc0c0c000001fffff, 0xc0c0c000003fffff, 0xf8f8f8fcfcffffff, 0x0000000000000000, 0xf0f0f0f1f1ffffff, 0xf0f0e0e0e0ffffff,
    0xe0e0c0c0c0ffffff, 0xc0c0808080ffffff, 0xc0c0000000ffffff, 0xc0c0000000ffffff, 0xfffcfcfcffffffff, 0xfff8f8f8ffffffff,
    0xfff1f1f1ffffffff, 0xf8e0e0e0f8ffffff, 0xf0c0c0c0f0ffffff, 0xe0808080e0ffffff, 0xc0000000c0ffffff, 0xc0000000c0ffffff,
    0xfcfcf8f8f8ffffff, 0xf8f8f8f8f8ffffff, 0xf1f1f0f8f8ffffff, 0xe0e0e0f0f0ffffff, 0xc0c0c0e0e0ffffff, 0x808080c0c0ffffff,
    0x000000c0c0ffffff, 0x000000c0c0ffffff, 0xfcf8f0f0f0ffffff, 0xf8f8f0f0f0ffffff, 0xf1f0f0f0f0ffffff, 0xe0e0e0e0f0ffffff,
    0xc0c0c0c0e0ffffff, 0x8080c0c0c0ffffff, 0x0000c0c0c0ffffff, 0x0000c0c0c0ffffff, 0xc0c0c0c0fffffcfc, 0xc0c0c0c0fffff8f8,
    0xc0c0c0c0fffff1f1, 0xc0c0c0c0ffffe3e3, 0xc0c0c0c0ffffc7c7, 0xc0c0c0c0ffff8f8f, 0xc0c0c0c0ffff1f1f, 0xc0c0c0c0ffff3f3f,
    0xc0c0c0c0fffcfcfc, 0xc0c0c0c0fff8f8f8, 0xc0c0c0c0fff1f1f1, 0xc0c0c0c0ffe3e3e3, 0xc0c0c0c0ffc7c7c7, 0xc0c0c0c0ff8f8f8f,
    0xc0c0c0c0ff1f1f1f, 0xc0c0c0c0ff3f3f3f, 0xe0e0e0f0fcfcfcff, 0xe0e0e0f1f8f8f8ff, 0xe0e0e0f1f1f1f1ff, 0xe0e0e0f1e3e3e3ff,
    0xc0c0c0c1c7c7c7ff, 0xc0c0c0c18f8f8fff, 0xc0c0c0c01f1f1fff, 0xc0c0c0c03f3f3fff, 0xf0f0f0f8fcfcffff, 0xf0f0f0f8f8f8ffff,
    0xf1f1f1f1f1f1ffff, 0xe1e1e1e3e3e3ffff, 0xe1e1e1c3c7c7ffff, 0xe1e1c1818f8fffff, 0xc0c0c0001f1fffff, 0xc0c0c0003f3fffff,
    0xfff9f0f8fcffffff, 0xfff9f0f8f8ffffff, 0x0000000000000000, 0xfff3e1e3e3ffffff, 0xfff3c1c3c7ffffff, 0xf1e181818fffffff,
    0xe0c000001fffffff, 0xc0c000003fffffff, 0xfff8f0f8ffffffff, 0xfff8f0f8ffffffff, 0xfff1f1f1ffffffff, 0xffe3e1e3ffffffff,
    0xffc3c1c3ffffffff, 0xf1818181ffffffff, 0xe0000000ffffffff, 0xc0000000ffffffff, 0xfcf8f0f9ffffffff, 0xf8f8f0fbffffffff,
    0xf1f1f1fbffffffff, 0xe3e3e1fbffffffff, 0xc7c3c1f3ffffffff, 0x818181e1ffffffff, 0x000000c0ffffffff, 0x000000c0ffffffff,
    0xf8f8f0f1ffffffff, 0xf8f8f1f1ffffffff, 0xf1f1f1f1ffffffff, 0xe3e3f1f1ffffffff, 0xc3c3e1f1ffffffff, 0x8181c1e1ffffffff,
    0x0000c0c0ffffffff, 0x0000c0c0ffffffff, 0x8080808080fffcfc, 0x8080808080fff8f8, 0x8080808080fff1f1, 0x8080808080ffe3e3,
    0x8080808080ffc7c7, 0x8080808080ff8f8f, 0x8080808080ff1f1f, 0x8080808080ff3f3f, 0x8080808080fcfcfc, 0x8080808080f8f8f8,
    0x8080808080f1f1f1, 0x8080808080e3e3e3, 0x8080808080c7c7c7, 0x80808080808f8f8f, 0x80808080801f1f1f, 0x80808080803f3f3f,
    0xc0c0c0c0e0fcfcff, 0xc0c0c0c0e0f8f8ff, 0xc0c0c0c0e0f1f1ff, 0xc0c0c0c0e0e3e3ff, 0x8080808080c7c7ff, 0x80808080808f8fff,
    0x80808080001f1fff, 0x80808080003f3fff, 0xe0e0e0f0f0fcffff, 0xe0e0e0f0f8f8ffff, 0xe0e0e0f1f1f1ffff, 0xc0c0c0c1e3e3ffff,
    0xc0c0c0c1c1c7ffff, 0xc0808080808fffff, 0x80808000001fffff, 0x80808000003fffff, 0xf0f0f0f0f0ffffff, 0xf0f0f0f8f8ffffff,
    0x0000000000000000, 0xe1e1e1e3e3ffffff, 0xe1e1c1c1c1ffffff, 0xc0c0808080ffffff, 0x8080000000ffffff, 0x8080000000ffffff,
    0xf1f0f0f0f1ffffff, 0xfff8f8f8ffffffff, 0xfff1f1f1ffffffff, 0xffe3e3e3ffffffff, 0xf1c1c1c1f1ffffff, 0xe0808080e0ffffff,
    0xc0000000c0ffffff, 0x8000000080ffffff, 0xf0f0f0f0f0ffffff, 0xf8f8f0f1f1ffffff, 0xf1f1f1f1f1ffffff, 0xe3e3e1f1f1ffffff,
    0xc1c1c1e1e1ffffff, 0x808080c0c0ffffff, 0x0000008080ffffff, 0x0000008080ffffff, 0xf0f0e0e0e0ffffff, 0xf8f0e0e0e0ffffff,
    0xf1f1e0e0e0ffffff, 0xe3e1e0e0e0ffffff, 0xc1c1c0c0e0ffffff, 0x80808080c0ffffff, 0x0000808080ffffff, 0x0000808080ffffff,
    0x80808080fffffcfc, 0x80808080fffff8f8, 0x80808080fffff1f1, 0x80808080ffffe3e3, 0x80808080ffffc7c7, 0x80808080ffff8f8f,
    0x80808080ffff1f1f, 0x80808080ffff3f3f, 0x80808080fffcfcfc, 0x80808080fff8f8f8, 0x80808080fff1f1f1, 0x80808080ffe3e3e3,
    0x80808080ffc7c7c7, 0x80808080ff8f8f8f, 0x80808080ff1f1f1f, 0x80808080ff3f3f3f, 0xc0c0c0e0fcfcfcff, 0xc0c0c0e0f8f8f8ff,
    0xc1c1c1e3f1f1f1ff, 0xc1c1c1e3e3e3e3ff, 0xc1c1c1e3c7c7c7ff, 0x818181838f8f8fff, 0x818181831f1f1fff, 0x818181813f3f3fff,
    0xe1e1e0e0fcfcffff, 0xe1e1e1f0f8f8ffff, 0xe1e1e1f1f1f1ffff, 0xe3e3e3e3e3e3ffff, 0xc3c3c3c7c7c7ffff, 0xc3c3c3878f8fffff,
    0xc3c383031f1fffff, 0x818181013f3fffff, 0xe3e1e0e0fcffffff, 0xfff3e0f0f8ffffff, 0xfff3e1f1f1ffffff, 0x0000000000000000,
    0xffe7c3c7c7ffffff, 0xffe783878fffffff, 0xe3c303031fffffff, 0xc18101013fffffff, 0xe3e0e0e0ffffffff, 0xfff0e0f0ffffffff,
    0xfff1e1f1ffffffff, 0xffe3e3e3ffffffff, 0xffc7c3c7ffffffff, 0xff878387ffffffff, 0xe3030303ffffffff, 0xc1010101ffffffff,
    0xe0e0e0e1ffffffff, 0xf8f0e0f3ffffffff, 0xf1f1e1f7ffffffff, 0xe3e3e3f7ffffffff, 0xc7c7c3f7ffffffff, 0x8f8783e7ffffffff,
    0x030303c3ffffffff, 0x01010181ffffffff, 0xe0e0e0e1ffffffff, 0xf0f0e1e3ffffffff, 0xf1f1e3e3ffffffff, 0xe3e3e3e3ffffffff,
    0xc7c7e3e3ffffffff, 0x8787c3e3ffffffff, 0x030383c3ffffffff, 0x01018181ffffffff, 0x0000000000fffcfc, 0x0000000000fff8f8,
    0x0000000000fff1f1, 0x0000000000ffe3e3, 0x0000000000ffc7c7, 0x0000000000ff8f8f, 0x0000000000ff1f1f, 0x0000000000ff3f3f,
    0x0000000000fcfcfc, 0x0000000000f8f8f8, 0x0000000000f1f1f1, 0x0000000000e3e3e3, 0x0000000000c7c7c7, 0x00000000008f8f8f,
    0x00000000001f1f1f, 0x00000000003f3f3f, 0x80808080c0fcfcff, 0x80808080c0f8f8ff, 0x80808080c1f1f1ff, 0x80808080c1e3e3ff,
    0x80808080c1c7c7ff, 0x00000000018f8fff, 0x00000000011f1fff, 0x00000000003f3fff, 0xc0c0c0c0c0fcffff, 0xc0c0c0e0e0f8ffff,
    0xc0c0c0e0f1f1ffff, 0xc1c1c1e3e3e3ffff, 0x81818183c7c7ffff, 0x81818183838fffff, 0x81010101011fffff, 0x00000000003fffff,
    0xc0c0c0c0c0ffffff, 0xe1e1e0e0e0ffffff, 0xe1e1e1f1f1ffffff, 0x0000000000000000, 0xc3c3c3c7c7ffffff, 0xc3c3838383ffffff,
    0x8181010101ffffff, 0x0000000000ffffff, 0xc1c0c0c0c1ffffff, 0xe3e0e0e0e3ffffff, 0xfff1f1f1ffffffff, 0xffe3e3e3ffffffff,
    0xffc7c7c7ffffffff, 0xe3838383e3ffffff, 0xc1010101c1ffffff, 0x8000000080ffffff, 0xc0c0c0c0c0ffffff, 0xe0e0e0e1e1ffffff,
    0xf1f1e1e3e3ffffff, 0xe3e3e3e3e3ffffff, 0xc7c7c3e3e3ffffff, 0x838383c3c3ffffff, 0x0101018181ffffff, 0x0000000000ffffff,
    0xc0c0c0c0c0ffffff, 0xe0e0c0c0c1ffffff, 0xf1e1c1c1c1ffffff, 0xe3e3c1c1c1ffffff, 0xc7c3c1c1c1ffffff, 0x83838181c1ffffff,
    0x0101010181ffffff, 0x0000000000ffffff, 0xf8f8f8fffffffcfc, 0xf8f8f8fffffff8f8, 0xf8f8f8fffffff1f1, 0xf8f8f8ffffffe3e3,
    0xf8f8f8ffffffc7c7, 0xf8f8f8ffffff8f8f, 0xf8f8f8ffffff1f1f, 0xf8f8f8ffffff3f3f, 0xf8f8f8fffffcfcfc, 0xf8f8f8fffff8f8f8,
    0xf8f8f8fffff1f1f1, 0xf8f8f8ffffe3e3e3, 0xf8f8f8ffffc7c7c7, 0xf8f8f8ffff8f8f8f, 0xf8f8f8ffff1f1f1f, 0xf8f8f8ffff3f3f3f,
    0xf8f8f8fffcfcfcff, 0xf8f8f8fff8f8f8ff, 0xf8f8f8fff1f1f1ff, 0xf8f8f8ffe3e3e3ff, 0xf8f8f8ffc7c7c7ff, 0xf8f8f8ff8f8f8fff,
    0xf8f8f8ff1f1f1fff, 0xf8f8f8ff3f3f3fff, 0xf8f8f8fcfcfcffff, 0xf8f8f8f8f8f8ffff, 0xf8f8f8f1f1f1ffff, 0xf8f8f8e3e3e3ffff,
    0xf8f8f8c7c7c7ffff, 0xf8f8f88f8f8fffff, 0xf8f8f81f1f1fffff, 0xf8f8f83f3f3fffff, 0xf8f8f8fcfcffffff, 0xf8f8f8f8f8ffffff,
    0xf8f8f0f1f1ffffff, 0xf8f8e0e3e3ffffff, 0xf8f8c0c7c7ffffff, 0xf8f8888f8fffffff, 0xf8f8181f1fffffff, 0xf8f8383f3fffffff,
    0x0000000000000000, 0xfcf8f8f8ffffffff, 0xfcf0f0f1ffffffff, 0xf8e0e0e3ffffffff, 0xf8c0c0c7ffffffff, 0xf888888fffffffff,
    0xf818181fffffffff, 0xf838383fffffffff, 0xf8f8fcffffffffff, 0xf8f8f8ffffffffff, 0xf0f0f0ffffffffff, 0xe0e0e0ffffffffff,
    0xc0c0c0ffffffffff, 0x888888ffffffffff, 0x181818ffffffffff, 0x383838ffffffffff, 0xf8f8fcffffffffff, 0xf8f8feffffffffff,
    0xf0f0fcffffffffff, 0xe0e0f8ffffffffff, 0xc0c0f8ffffffffff, 0x8888f8ffffffffff, 0x1818f8ffffffffff, 0x3838f8ffffffffff,
    0xf0f0f0f0fffffcfc, 0xf0f0f0f0fffff8f8, 0xf0f0f0f0fffff1f1, 0xf0f0f0f0ffffe3e3, 0xf0f0f0f0ffffc7c7, 0xf0f0f0f0ffff8f8f,
    0xf0f0f0f0ffff1f1f, 0xf0f0f0f0ffff3f3f, 0xf0f0f0f0fffcfcfc, 0xf0f0f0f0fff8f8f8, 0xf0f0f0f0fff1f1f1, 0xf0f0f0f0ffe3e3e3,
    0xf0f0f0f0ffc7c7c7, 0xf0f0f0f0ff8f8f8f, 0xf0f0f0f0ff1f1f1f, 0xf0f0f0f0ff3f3f3f, 0xf0f0f0f0fcfcfcff, 0xf0f0f0f0f8f8f8ff,
    0xf0f0f0f0f1f1f1ff, 0xf0f0f0f0e3e3e3ff, 0xf0f0f0f0c7c7c7ff, 0xf0f0f0f08f8f8fff, 0xf0f0f0f01f1f1fff, 0xf0f0f0f03f3f3fff,
    0xf0f0f0f0fcfcffff, 0xf0f0f0f0f8f8ffff, 0xf0f0f0f0f1f1ffff, 0xf0f0f0e0e3e3ffff, 0xf0f0f0c0c7c7ffff, 0xf0f0f0808f8fffff,
    0xf0f0f0101f1fffff, 0xf0f0f0303f3fffff, 0xf0f0f0f0fcffffff, 0xf0f0f0f8f8ffffff, 0xf0f0f0f0f1ffffff, 0xf0f0e0e0e3ffffff,
    0xf0f0c0c0c7ffffff, 0xf0f080808fffffff, 0xf0f010101fffffff, 0xf0f030303fffffff, 0x0000000000000000, 0xf8f8f8f8ffffffff,
    0xf8f0f0f0ffffffff, 0xf0e0e0e0ffffffff, 0xf0c0c0c0ffffffff, 0xf0808080ffffffff, 0xf0101010ffffffff, 0xf0303030ffffffff,
    0xf0f0f0ffffffffff, 0xf8f8f8ffffffffff, 0xf0f0f0fcffffffff, 0xe0e0e0f8ffffffff, 0xc0c0c0f0ffffffff, 0x808080f0ffffffff,
    0x101010f0ffffffff, 0x303030f0ffffffff, 0xf0f0f0f8ffffffff, 0xf8f8fcfcffffffff, 0xf0f0f8f8ffffffff, 0xe0e0f0f0ffffffff,
    0xc0c0f0f0ffffffff, 0x8080f0f0ffffffff, 0x1010f0f0ffffffff, 0x3030f0f0ffffffff, 0xf0f0f0fffffffcfc, 0xf0f0f0fffffff8f8,
    0xf0f0f0fffffff1f1, 0xf0f0f0ffffffe3e3, 0xf0f0f0ffffffc7c7, 0xf0f0f0ffffff8f8f, 0xf0f0f0ffffff1f1f, 0xf0f0f0ffffff3f3f,
    0xf0f0f0fffffcfcfc, 0xf0f0f0fffff8f8f8, 0xf0f0f0fffff1f1f1, 0xf0f0f0ffffe3e3e3, 0xf0f0f0ffffc7c7c7, 0xf0f0f0ffff8f8f8f,
    0xf0f0f0ffff1f1f1f, 0xf0f0f0ffff3f3f3f, 0xf0f0f0fffcfcfcff, 0xf0f0f0fff8f8f8ff, 0xf0f0f0fff1f1f1ff, 0xf0f0f0ffe3e3e3ff,
    0xf0f0f0ffc7c7c7ff, 0xf0f0f0ff8f8f8fff, 0xf0f0f0ff1f1f1fff, 0xf0f0f0ff3f3f3fff, 0xf8f8f8fcfcfcffff, 0xf8f8f8f8f8f8ffff,
    0xf8f8f8f1f1f1ffff, 0xf8f8f0e3e3e3ffff, 0xf8f8f0c7c7c7ffff, 0xf0f0f08f8f8fffff, 0xf0f0f01f1f1fffff, 0xf0f0f03f3f3fffff,
    0xfdf8fcfcfcffffff, 0xfdf8f8f8f8ffffff, 0xfdf8f1f1f1ffffff, 0xfdf8e1e3e3ffffff, 0xf8f8c0c7c7ffffff, 0xf0f0808f8fffffff,
    0xf0f0101f1fffffff, 0xf0f0303f3fffffff, 0xfef8fcfcffffffff, 0x0000000000000000, 0xfaf0f1f1ffffffff, 0xfde0e1e3ffffffff,
    0xf8c0c0c7ffffffff, 0xf080808fffffffff, 0xf010101fffffffff, 0xf030303fffffffff, 0xfcf8fcffffffffff, 0xf8f8f8ffffffffff,
    0xf1f0f1ffffffffff, 0xe1e0e1ffffffffff, 0xc0c0c0ffffffffff, 0x808080ffffffffff, 0x101010ffffffffff, 0x303030ffffffffff,
    0xfcf8fdffffffffff, 0xf8f8fdffffffffff, 0xf1f0fdffffffffff, 0xe0e0f9ffffffffff, 0xc0c0f0ffffffffff, 0x8080f0ffffffffff,
    0x1010f0ffffffffff, 0x3030f0ffffffffff, 0xe0e0e0e0fffffcfc, 0xe0e0e0e0fffff8f8, 0xe0e0e0e0fffff1f1, 0xe0e0e0e0ffffe3e3,
    0xe0e0e0e0ffffc7c7, 0xe0e0e0e0ffff8f8f, 0xe0e0e0e0ffff1f1f, 0xe0e0e0e0ffff3f3f, 0xe0e0e0e0fffcfcfc, 0xe0e0e0e0fff8f8f8,
    0xe0e0e0e0fff1f1f1, 0xe0e0e0e0ffe3e3e3, 0xe0e0e0e0ffc7c7c7, 0xe0e0e0e0ff8f8f8f, 0xe0e0e0e0ff1f1f1f, 0xe0e0e0e0ff3f3f3f,
    0xe0e0e0e0fcfcfcff, 0xe0e0e0e0f8f8f8ff, 0xe0e0e0e0f1f1f1ff, 0xe0e0e0e0e3e3e3ff, 0xe0e0e0e0c7c7c7ff, 0xe0e0e0e08f8f8fff,
    0xe0e0e0e01f1f1fff, 0xe0e0e0e03f3f3fff, 0xf0f0f0f0fcfcffff, 0xf0f0f0f0f8f8ffff, 0xf0f0f0f0f1f1ffff, 0xf0e0e0e0e3e3ffff,
    0xf0e0e0c0c7c7ffff, 0xe0e0e0808f8fffff, 0xe0e0e0001f1fffff, 0xe0e0e0203f3fffff, 0xf8f8f8fcfcffffff, 0xf8f8f8f8f8ffffff,
    0xf8f8f0f1f1ffffff, 0xf8f8e0e0e3ffffff, 0xf0f0c0c0c7ffffff, 0xe0e080808fffffff, 0xe0e000001fffffff, 0xe0e020203fffffff,
    0xfdfcfcfcffffffff, 0x0000000000000000, 0xf5f0f1f1ffffffff, 0xf8e0e0e0ffffffff, 0xf0c0c0c0ffffffff, 0xe0808080ffffffff,
    0xe0000000ffffffff, 0xe0202020ffffffff, 0xfcfcfcffffffffff, 0xf8f8f8ffffffffff, 0xf0f1f1ffffffffff, 0xe0e0e0f8ffffffff,
    0xc0c0c0f0ffffffff, 0x808080e0ffffffff, 0x000000e0ffffffff, 0x202020e0ffffffff, 0xfcf8f8f8ffffffff, 0xf8f8f8f8ffffffff,
    0xf0f0f8f8ffffffff, 0xe0e0f0f0ffffffff, 0xc0c0e0e0ffffffff, 0x8080e0e0ffffffff, 0x0000e0e0ffffffff, 0x2020e0e0ffffffff,
    0xe0e0e0fffffffcfc, 0xe0e0e0fffffff8f8, 0xe0e0e0fffffff1f1, 0xe0e0e0ffffffe3e3, 0xe0e0e0ffffffc7c7, 0xe0e0e0ffffff8f8f,
    0xe0e0e0ffffff1f1f, 0xe0e0e0ffffff3f3f, 0xe0e0e0fffffcfcfc, 0xe0e0e0fffff8f8f8, 0xe0e0e0fffff1f1f1, 0xe0e0e0ffffe3e3e3,
    0xe0e0e0ffffc7c7c7, 0xe0e0e0ffff8f8f8f, 0xe0e0e0ffff1f1f1f, 0xe0e0e0ffff3f3f3f, 0xe0e0e0fffcfcfcff, 0xe0e0e0fff8f8f8ff,
    0xe0e0e0fff1f1f1ff, 0xe0e0e0ffe3e3e3ff, 0xe0e0e0ffc7c7c7ff, 0xe0e0e0ff8f8f8fff, 0xe0e0e0ff1f1f1fff, 0xe0e0e0ff3f3f3fff,
    0xf1f1f0fcfcfcffff, 0xf1f1f1f8f8f8ffff, 0xf1f1f1f1f1f1ffff, 0xf1f1f1e3e3e3ffff, 0xf1f1e1c7c7c7ffff, 0xf1f1e18f8f8fffff,
    0xe0e0e01f1f1fffff, 0xe0e0e03f3f3fffff, 0xfbf1f8fcfcffffff, 0xfbf1f8f8f8ffffff, 0xfbf1f1f1f1ffffff, 0xfbf1e3e3e3ffffff,
    0xfbf1c3c7c7ffffff, 0xf1f1818f8fffffff, 0xe0e0001f1fffffff, 0xe0e0203f3fffffff, 0xfbf0f8fcffffffff, 0xfdf0f8f8ffffffff,
    0x0000000000000000, 0xf7e1e3e3ffffffff, 0xfbc1c3c7ffffffff, 0xf181818fffffffff, 0xe000001fffffffff, 0xe020203fffffffff,
    0xf8f0f8ffffffffff, 0xf8f0f8ffffffffff, 0xf1f1f1ffffffffff, 0xe3e1e3ffffffffff, 0xc3c1c3ffffffffff, 0x818181ffffffffff,
    0x000000ffffffffff, 0x202020ffffffffff, 0xf8f0f9ffffffffff, 0xf8f0fbffffffffff, 0xf1f1fbffffffffff, 0xe3e1fbffffffffff,
    0xc3c1f3ffffffffff, 0x8181e1ffffffffff, 0x0000e0ffffffffff, 0x2020e0ffffffffff, 0xc0c0c0c0fffffcfc, 0xc0c0c0c0fffff8f8,
    0xc0c0c0c0fffff1f1, 0xc0c0c0c0ffffe3e3, 0xc0c0c0c0ffffc7c7, 0xc0c0c0c0ffff8f8f, 0xc0c0c0c0ffff1f1f, 0xc0c0c0c0ffff3f3f,
    0xc0c0c0c0fffcfcfc, 0xc0c0c0c0fff8f8f8, 0xc0c0c0c0fff1f1f1, 0xc0c0c0c0ffe3e3e3, 0xc0c0c0c0ffc7c7c7, 0xc0c0c0c0ff8f8f8f,
    0xc0c0c0c0ff1f1f1f, 0xc0c0c0c0ff3f3f3f, 0xc0c0c0c0fcfcfcff, 0xc0c0c0c0f8f8f8ff, 0xc0c0c0c0f1f1f1ff, 0xc0c0c0c0e3e3e3ff,
    0xc0c0c0c0c7c7c7ff, 0xc0c0c0c08f8f8fff, 0xc0c0c0c01f1f1fff, 0xc0c0c0c03f3f3fff, 0xe0e0e0e0fcfcffff, 0xe0e0e0e0f8f8ffff,
    0xe0e0e0e0f1f1ffff, 0xe0e0e0e0e3e3ffff, 0xe0c0c0c0c7c7ffff, 0xe0c0c0808f8fffff, 0xc0c0c0001f1fffff, 0xc0c0c0003f3fffff,
    0xf1f1f0f0fcffffff, 0xf1f1f0f8f8ffffff, 0xf1f1f1f1f1ffffff, 0xf1f1e1e3e3ffffff, 0xf1f1c1c1c7ffffff, 0xe0e080808fffffff,
    0xc0c000001fffffff, 0xc0c000003fffffff, 0xf1f0f0f0ffffffff, 0xfaf8f8f8ffffffff, 0x0000000000000000, 0xebe3e3e3ffffffff,
    0xf1c1c1c1ffffffff, 0xe0808080ffffffff, 0xc0000000ffffffff, 0xc0000000ffffffff, 0xf0f0f0f1ffffffff, 0xf8f8f8ffffffffff,
    0xf1f1f1ffffffffff, 0xe3e3e3ffffffffff, 0xc1c1c1f1ffffffff, 0x808080e0ffffffff, 0x000000c0ffffffff, 0x000000c0ffffffff,
    0xf0f0f0f0ffffffff, 0xf8f0f1f1ffffffff, 0xf1f1f1f1ffffffff, 0xe3e1f1f1ffffffff, 0xc1c1e1e1ffffffff, 0x8080c0c0ffffffff,
    0x0000c0c0ffffffff, 0x0000c0c0ffffffff, 0xc1c1c1fffffffcfc, 0xc1c1c1fffffff8f8, 0xc1c1c1fffffff1f1, 0xc1c1c1ffffffe3e3,
    0xc1c1c1ffffffc7c7, 0xc1c1c1ffffff8f8f, 0xc1c1c1ffffff1f1f, 0xc1c1c1ffffff3f3f, 0xc1c1c1fffffcfcfc, 0xc1c1c1fffff8f8f8,
    0xc1c1c1fffff1f1f1, 0xc1c1c1ffffe3e3e3, 0xc1c1c1ffffc7c7c7, 0xc1c1c1ffff8f8f8f, 0xc1c1c1ffff1f1f1f, 0xc1c1c1ffff3f3f3f,
    0xc1c1c1fffcfcfcff, 0xc1c1c1fff8f8f8ff, 0xc1c1c1fff1f1f1ff, 0xc1c1c1ffe3e3e3ff, 0xc1c1c1ffc7c7c7ff, 0xc1c1c1ff8f8f8fff,
    0xc1c1c1ff1f1f1fff, 0xc1c1c1ff3f3f3fff, 0xe3e3e1fcfcfcffff, 0xe3e3e1f8f8f8ffff, 0xe3e3e3f1f1f1ffff, 0xe3e3e3e3e3e3ffff,
    0xe3e3e3c7c7c7ffff, 0xe3e3c38f8f8fffff, 0xe3e3c31f1f1fffff, 0xc1c1c13f3f3fffff, 0xe3e3e0fcfcffffff, 0xf7e3f0f8f8ffffff,
    0xf7e3f1f1f1ffffff, 0xf7e3e3e3e3ffffff, 0xf7e3c7c7c7ffffff, 0xf7e3878f8fffffff, 0xe3e3031f1fffffff, 0xc1c1013f3fffffff,
    0xe3e0e0fcffffffff, 0xf7e0f0f8ffffffff, 0xfbe1f1f1ffffffff, 0x0000000000000000, 0xefc3c7c7ffffffff, 0xf783878fffffffff,
    0xe303031fffffffff, 0xc101013fffffffff, 0xe0e0e0ffffffffff, 0xf0e0f0ffffffffff, 0xf1e1f1ffffffffff, 0xe3e3e3ffffffffff,
    0xc7c3c7ffffffffff, 0x878387ffffffffff, 0x030303ffffffffff, 0x010101ffffffffff, 0xe0e0e1ffffffffff, 0xf0e0f3ffffffffff,
    0xf1e1f7ffffffffff, 0xe3e3f7ffffffffff, 0xc7c3f7ffffffffff, 0x8783e7ffffffffff, 0x0303c3ffffffffff, 0x0101c1ffffffffff,
    0x80808080fffffcfc, 0x80808080fffff8f8, 0x80808080fffff1f1, 0x80808080ffffe3e3, 0x80808080ffffc7c7, 0x80808080ffff8f8f,
    0x80808080ffff1f1f, 0x80808080ffff3f3f, 0x80808080fffcfcfc, 0x80808080fff8f8f8, 0x80808080fff1f1f1, 0x80808080ffe3e3e3,
    0x80808080ffc7c7c7, 0x80808080ff8f8f8f, 0x80808080ff1f1f1f, 0x80808080ff3f3f3f, 0x80808080fcfcfcff, 0x80808080f8f8f8ff,
    0x80808080f1f1f1ff, 0x80808080e3e3e3ff, 0x80808080c7c7c7ff, 0x808080808f8f8fff, 0x808080801f1f1fff, 0x808080803f3f3fff,
    0xc1c0c0c0fcfcffff, 0xc1c0c0c0f8f8ffff, 0xc1c1c1c1f1f1ffff, 0xc1c1c1c1e3e3ffff, 0xc1c1c1c1c7c7ffff, 0xc18181818f8fffff,
    0xc18181011f1fffff, 0x808080003f3fffff, 0xc1c1c0c0fcffffff, 0xe3e3e0e0f8ffffff, 0xe3e3e1f1f1ffffff, 0xe3e3e3e3e3ffffff,
    0xe3e3c3c7c7ffffff, 0xe3e383838fffffff, 0xc1c101011fffffff, 0x808000003fffffff, 0xc1c0c0c0ffffffff, 0xe3e0e0e0ffffffff,
    0xf5f1f1f1ffffffff, 0x0000000000000000, 0xd7c7c7c7ffffffff, 0xe3838383ffffffff, 0xc1010101ffffffff, 0x80000000ffffffff,
    0xc0c0c0c1ffffffff, 0xe0e0e0e3ffffffff, 0xf1f1f1ffffffffff, 0xe3e3e3ffffffffff, 0xc7c7c7ffffffffff, 0x838383e3ffffffff,
    0x010101c1ffffffff, 0x00000080ffffffff, 0xc0c0c0c0ffffffff, 0xe0e0e1e1ffffffff, 0xf1e1e3e3ffffffff, 0xe3e3e3e3ffffffff,
    0xc7c3e3e3ffffffff, 0x8383c3c3ffffffff, 0x01018181ffffffff, 0x00008080ffffffff, 0xfcfcfffffffffcfc, 0xfcfcfffffffff8f8,
    0xfcfcfffffffff1f1, 0xfcfcffffffffe3e3, 0xfcfcffffffffc7c7, 0xfcfcffffffff8f8f, 0xfcfcffffffff1f1f, 0xfcfcffffffff3f3f,
    0xfcfcfffffffcfcfc, 0xfcfcfffffff8f8f8, 0xfcfcfffffff1f1f1, 0xfcfcffffffe3e3e3, 0xfcfcffffffc7c7c7, 0xfcfcffffff8f8f8f,
    0xfcfcffffff1f1f1f, 0xfcfcffffff3f3f3f, 0xfcfcfffffcfcfcff, 0xfcfcfffff8f8f8ff, 0xfcfcfffff1f1f1ff, 0xfcfcffffe3e3e3ff,
    0xfcfcffffc7c7c7ff, 0xfcfcffff8f8f8fff, 0xfcfcffff1f1f1fff, 0xfcfcffff3f3f3fff, 0xfcfcfffcfcfcffff, 0xfcfcfff8f8f8ffff,
    0xfcfcfff1f1f1ffff, 0xfcfcffe3e3e3ffff, 0xfcfcffc7c7c7ffff, 0xfcfcff8f8f8fffff, 0xfcfcff1f1f1fffff, 0xfcfcff3f3f3fffff,
    0xfcfcfcfcfcffffff, 0xfcfcf8f8f8ffffff, 0xfcfcf1f1f1ffffff, 0xfcfce3e3e3ffffff, 0xfcfcc7c7c7ffffff, 0xfcfc8f8f8fffffff,
    0xfcfc1f1f1fffffff, 0xfcfc3f3f3fffffff, 0xfcfcfcfcffffffff, 0xfcf8f8f8ffffffff, 0xfcf0f1f1ffffffff, 0xfce0e3e3ffffffff,
    0xfcc4c7c7ffffffff, 0xfc8c8f8fffffffff, 0xfc1c1f1fffffffff, 0xfc3c3f3fffffffff, 0x0000000000000000, 0xf8f8f8ffffffffff,
    0xf0f0f1ffffffffff, 0xe0e0e3ffffffffff, 0xc4c4c7ffffffffff, 0x8c8c8fffffffffff, 0x1c1c1fffffffffff, 0x3c3c3fffffffffff,
    0xf8f8ffffffffffff, 0xf8f8ffffffffffff, 0xf0f0ffffffffffff, 0xe0e0ffffffffffff, 0xc4c4ffffffffffff, 0x8c8cffffffffffff,
    0x1c1cffffffffffff, 0x3c3cffffffffffff, 0xf8f8f8fffffffcfc, 0xf8f8f8fffffff8f8, 0xf8f8f8fffffff1f1, 0xf8f8f8ffffffe3e3,
    0xf8f8f8ffffffc7c7, 0xf8f8f8ffffff8f8f, 0xf8f8f8ffffff1f1f, 0xf8f8f8ffffff3f3f, 0xf8f8f8fffffcfcfc, 0xf8f8f8fffff8f8f8,
    0xf8f8f8fffff1f1f1, 0xf8f8f8ffffe3e3e3, 0xf8f8f8ffffc7c7c7, 0xf8f8f8ffff8f8f8f, 0xf8f8f8ffff1f1f1f, 0xf8f8f8ffff3f3f3f,
    0xf8f8f8fffcfcfcff, 0xf8f8f8fff8f8f8ff, 0xf8f8f8fff1f1f1ff, 0xf8f8f8ffe3e3e3ff, 0xf8f8f8ffc7c7c7ff, 0xf8f8f8ff8f8f8fff,
    0xf8f8f8ff1f1f1fff, 0xf8f8f8ff3f3f3fff, 0xf8f8f8fcfcfcffff, 0xf8f8f8f8f8f8ffff, 0xf8f8f8f1f1f1ffff, 0xf8f8f8e3e3e3ffff,
    0xf8f8f8c7c7c7ffff, 0xf8f8f88f8f8fffff, 0xf8f8f81f1f1fffff, 0xf8f8f83f3f3fffff, 0xf8f8f8fcfcffffff, 0xf8f8f8f8f8ffffff,
    0xf8f8f0f1f1ffffff, 0xf8f8e0e3e3ffffff, 0xf8f8c0c7c7ffffff, 0xf8f8888f8fffffff, 0xf8f8181f1fffffff, 0xf8f8383f3fffffff,
    0xfcfcfcfcffffffff, 0xfcf8f8f8ffffffff, 0xfcf0f0f1ffffffff, 0xf8e0e0e3ffffffff, 0xf8c0c0c7ffffffff, 0xf888888fffffffff,
    0xf818181fffffffff, 0xf838383fffffffff, 0x0000000000000000, 0xf8f8f8ffffffffff, 0xf0f0f0ffffffffff, 0xe0e0e0ffffffffff,
    0xc0c0c0ffffffffff, 0x888888ffffffffff, 0x181818ffffffffff, 0x383838ffffffffff, 0xf0f0f1ffffffffff, 0xf8f8ffffffffffff,
    0xf0f0fcffffffffff, 0xe0e0f8ffffffffff, 0xc0c0f8ffffffffff, 0x8888f8ffffffffff, 0x1818f8ffffffffff, 0x3838f8ffffffffff,
    0xf8f8fffffffffcfc, 0xf8f8fffffffff8f8, 0xf8f8fffffffff1f1, 0xf8f8ffffffffe3e3, 0xf8f8ffffffffc7c7, 0xf8f8ffffffff8f8f,
    0xf8f8ffffffff1f1f, 0xf8f8ffffffff3f3f, 0xf8f8fffffffcfcfc, 0xf8f8fffffff8f8f8, 0xf8f8fffffff1f1f1, 0xf8f8ffffffe3e3e3,
    0xf8f8ffffffc7c7c7, 0xf8f8ffffff8f8f8f, 0xf8f8ffffff1f1f1f, 0xf8f8ffffff3f3f3f, 0xf8f8fffffcfcfcff, 0xf8f8fffff8f8f8ff,
    0xf8f8fffff1f1f1ff, 0xf8f8ffffe3e3e3ff, 0xf8f8ffffc7c7c7ff, 0xf8f8ffff8f8f8fff, 0xf8f8ffff1f1f1fff, 0xf8f8ffff3f3f3fff,
    0xf8f8fffcfcfcffff, 0xf8f8fff8f8f8ffff, 0xf8f8fff1f1f1ffff, 0xf8f8ffe3e3e3ffff, 0xf8f8ffc7c7c7ffff, 0xf8f8ff8f8f8fffff,
    0xf8f8ff1f1f1fffff, 0xf8f8ff3f3f3fffff, 0xfaf8fcfcfcffffff, 0xfaf8f8f8f8ffffff, 0xfaf8f1f1f1ffffff, 0xfaf8e3e3e3ffffff,
    0xf8f8c7c7c7ffffff, 0xf8f88f8f8fffffff, 0xf8f81f1f1fffffff, 0xf8f83f3f3fffffff, 0xf8fcfcfcffffffff, 0xfaf8f8f8ffffffff,
    0xf8f1f1f1ffffffff, 0xfae1e3e3ffffffff, 0xf8c0c7c7ffffffff, 0xf8888f8fffffffff, 0xf8181f1fffffffff, 0xf8383f3fffffffff,
    0xf8fcfcffffffffff, 0x0000000000000000, 0xf0f1f1ffffffffff, 0xe2e1e3ffffffffff, 0xc0c0c7ffffffffff, 0x88888fffffffffff,
    0x18181fffffffffff, 0x38383fffffffffff, 0xf8fcffffffffffff, 0xf8f8ffffffffffff, 0xf0f1ffffffffffff, 0xe0e1ffffffffffff,
    0xc0c0ffffffffffff, 0x8888ffffffffffff, 0x1818ffffffffffff, 0x3838ffffffffffff, 0xf0f0f0fffffffcfc, 0xf0f0f0fffffff8f8,
    0xf0f0f0fffffff1f1, 0xf0f0f0ffffffe3e3, 0xf0f0f0ffffffc7c7, 0xf0f0f0ffffff8f8f, 0xf0f0f0ffffff1f1f, 0xf0f0f0ffffff3f3f,
    0xf0f0f0fffffcfcfc, 0xf0f0f0fffff8f8f8, 0xf0f0f0fffff1f1f1, 0xf0f0f0ffffe3e3e3, 0xf0f0f0ffffc7c7c7, 0xf0f0f0ffff8f8f8f,
    0xf0f0f0ffff1f1f1f, 0xf0f0f0ffff3f3f3f, 0xf0f0f0fffcfcfcff, 0xf0f0f0fff8f8f8ff, 0xf0f0f0fff1f1f1ff, 0xf0f0f0ffe3e3e3ff,
    0xf0f0f0ffc7c7c7ff, 0xf0f0f0ff8f8f8fff, 0xf0f0f0ff1f1f1fff, 0xf0f0f0ff3f3f3fff, 0xf0f0f0fcfcfcffff, 0xf0f0f0f8f8f8ffff,
    0xf0f0f0f1f1f1ffff, 0xf0f0f0e3e3e3ffff, 0xf0f0f0c7c7c7ffff, 0xf0f0f08f8f8fffff, 0xf0f0f01f1f1fffff, 0xf0f0f03f3f3fffff,
    0xf0f0f0fcfcffffff, 0xf0f0f0f8f8ffffff, 0xf0f0f0f1f1ffffff, 0xf0f0e0e3e3ffffff, 0xf0f0c0c7c7ffffff, 0xf0f0808f8fffffff,
    0xf0f0101f1fffffff, 0xf0f0303f3fffffff, 0xfaf8fcfcffffffff, 0xfdf8f8f8ffffffff, 0xfaf0f1f1ffffffff, 0xf8e0e0e3ffffffff,
    0xf0c0c0c7ffffffff, 0xf080808fffffffff, 0xf010101fffffffff, 0xf030303fffffffff, 0xfcfcfcffffffffff, 0x0000000000000000,
    0xf1f1f1ffffffffff, 0xe0e0e0ffffffffff, 0xc0c0c0ffffffffff, 0x808080ffffffffff, 0x101010ffffffffff, 0x303030ffffffffff,
    0xfcfcffffffffffff, 0xf8f8ffffffffffff, 0xf1f1ffffffffffff, 0xe0e0f8ffffffffff, 0xc0c0f0ffffffffff, 0x8080f0ffffffffff,
    0x1010f0ffffffffff, 0x3030f0ffffffffff, 0xf1f1fffffffffcfc, 0xf1f1fffffffff8f8, 0xf1f1fffffffff1f1, 0xf1f1ffffffffe3e3,
    0xf1f1ffffffffc7c7, 0xf1f1ffffffff8f8f, 0xf1f1ffffffff1f1f, 0xf1f1ffffffff3f3f, 0xf1f1fffffffcfcfc, 0xf1f1fffffff8f8f8,
    0xf1f1fffffff1f1f1, 0xf1f1ffffffe3e3e3, 0xf1f1ffffffc7c7c7, 0xf1f1ffffff8f8f8f, 0xf1f1ffffff1f1f1f, 0xf1f1ffffff3f3f3f,
    0xf1f1fffffcfcfcff, 0xf1f1fffff8f8f8ff, 0xf1f1fffff1f1f1ff, 0xf1f1ffffe3e3e3ff, 0xf1f1ffffc7c7c7ff, 0xf1f1ffff8f8f8fff,
    0xf1f1ffff1f1f1fff, 0xf1f1ffff3f3f3fff, 0xf1f1fffcfcfcffff, 0xf1f1fff8f8f8ffff, 0xf1f1fff1f1f1ffff, 0xf1f1ffe3e3e3ffff,
    0xf1f1ffc7c7c7ffff, 0xf1f1ff8f8f8fffff, 0xf1f1ff1f1f1fffff, 0xf1f1ff3f3f3fffff, 0xf5f1fcfcfcffffff, 0xf5f1f8f8f8ffffff,
    0xf5f1f1f1f1ffffff, 0xf5f1e3e3e3ffffff, 0xf5f1c7c7c7ffffff, 0xf1f18f8f8fffffff, 0xf1f11f1f1fffffff, 0xf1f13f3f3fffffff,
    0xf5f8fcfcffffffff, 0xf1f8f8f8ffffffff, 0xf5f1f1f1ffffffff, 0xf1e3e3e3ffffffff, 0xf5c3c7c7ffffffff, 0xf1818f8fffffffff,
    0xf1111f1fffffffff, 0xf1313f3fffffffff, 0xf4f8fcffffffffff, 0xf0f8f8ffffffffff, 0x0000000000000000, 0xe1e3e3ffffffffff,
    0xc5c3c7ffffffffff, 0x81818fffffffffff, 0x11111fffffffffff, 0x31313fffffffffff, 0xf0f8ffffffffffff, 0xf0f8ffffffffffff,
    0xf1f1ffffffffffff, 0xe1e3ffffffffffff, 0xc1c3ffffffffffff, 0x8181ffffffffffff, 0x1111ffffffffffff, 0x3131ffffffffffff,
    0xe0e0e0fffffffcfc, 0xe0e0e0fffffff8f8, 0xe0e0e0fffffff1f1, 0xe0e0e0ffffffe3e3, 0xe0e0e0ffffffc7c7, 0xe0e0e0ffffff8f8f,
    0xe0e0e0ffffff1f1f, 0xe0e0e0ffffff3f3f, 0xe0e0e0fffffcfcfc, 0xe0e0e0fffff8f8f8, 0xe0e0e0fffff1f1f1, 0xe0e0e0ffffe3e3e3,
    0xe0e0e0ffffc7c7c7, 0xe0e0e0ffff8f8f8f, 0xe0e0e0ffff1f1f1f, 0xe0e0e0ffff3f3f3f, 0xe0e0e0fffcfcfcff, 0xe0e0e0fff8f8f8ff,
    0xe0e0e0fff1f1f1ff, 0xe0e0e0ffe3e3e3ff, 0xe0e0e0ffc7c7c7ff, 0xe0e0e0ff8f8f8fff, 0xe0e0e0ff1f1f1fff, 0xe0e0e0ff3f3f3fff,
    0xe0e0e0fcfcfcffff, 0xe0e0e0f8f8f8ffff, 0xe0e0e0f1f1f1ffff, 0xe0e0e0e3e3e3ffff, 0xe0e0e0c7c7c7ffff, 0xe0e0e08f8f8fffff,
    0xe0e0e01f1f1fffff, 0xe0e0e03f3f3fffff, 0xe0e0e0fcfcffffff, 0xe0e0e0f8f8ffffff, 0xe0e0e0f1f1ffffff, 0xe0e0e0e3e3ffffff,
    0xe0e0c0c7c7ffffff, 0xe0e0808f8fffffff, 0xe0e0001f1fffffff, 0xe0e0203f3fffffff, 0xf0f0f0fcffffffff, 0xf4f0f8f8ffffffff,
    0xfbf1f1f1ffffffff, 0xf5e1e3e3ffffffff, 0xf1c1c1c7ffffffff, 0xe080808fffffffff, 0xe000001fffffffff, 0xe020203fffffffff,
    0xf0f0f0ffffffffff, 0xf8f8f8ffffffffff, 0x0000000000000000, 0xe3e3e3ffffffffff, 0xc1c1c1ffffffffff, 0x808080ffffffffff,
    0x000000ffffffffff, 0x202020ffffffffff, 0xf0f0f1ffffffffff, 0xf8f8ffffffffffff, 0xf1f1ffffffffffff, 0xe3e3ffffffffffff,
    0xc1c1f1ffffffffff, 0x8080e0ffffffffff, 0x0000e0ffffffffff, 0x2020e0ffffffffff, 0xe3e3fffffffffcfc, 0xe3e3fffffffff8f8,
    0xe3e3fffffffff1f1, 0xe3e3ffffffffe3e3, 0xe3e3ffffffffc7c7, 0xe3e3ffffffff8f8f, 0xe3e3ffffffff1f1f, 0xe3e3ffffffff3f3f,
    0xe3e3fffffffcfcfc, 0xe3e3fffffff8f8f8, 0xe3e3fffffff1f1f1, 0xe3e3ffffffe3e3e3, 0xe3e3ffffffc7c7c7, 0xe3e3ffffff8f8f8f,
    0xe3e3ffffff1f1f1f, 0xe3e3ffffff3f3f3f, 0xe3e3fffffcfcfcff, 0xe3e3fffff8f8f8ff, 0xe3e3fffff1f1f1ff, 0xe3e3ffffe3e3e3ff,
    0xe3e3ffffc7c7c7ff, 0xe3e3ffff8f8f8fff, 0xe3e3ffff1f1f1fff, 0xe3e3ffff3f3f3fff, 0xe3e3fffcfcfcffff, 0xe3e3fff8f8f8ffff,
    0xe3e3fff1f1f1ffff, 0xe3e3ffe3e3e3ffff, 0xe3e3ffc7c7c7ffff, 0xe3e3ff8f8f8fffff, 0xe3e3ff1f1f1fffff, 0xe3e3ff3f3f3fffff,
    0xe3e3fcfcfcffffff, 0xebe3f8f8f8ffffff, 0xebe3f1f1f1ffffff, 0xebe3e3e3e3ffffff, 0xebe3c7c7c7ffffff, 0xebe38f8f8fffffff,
    0xe3e31f1f1fffffff, 0xe3e33f3f3fffffff, 0xe3e0fcfcffffffff, 0xebf0f8f8ffffffff, 0xe3f1f1f1ffffffff, 0xebe3e3e3ffffffff,
    0xe3c7c7c7ffffffff, 0xeb878f8fffffffff, 0xe3031f1fffffffff, 0xe3233f3fffffffff, 0xe0e0fcffffffffff, 0xe8f0f8ffffffffff,
    0xe1f1f1ffffffffff, 0x0000000000000000, 0xc3c7c7ffffffffff, 0x8b878fffffffffff, 0x03031fffffffffff, 0x23233fffffffffff,
    0xe0e0ffffffffffff, 0xe0f0ffffffffffff, 0xe1f1ffffffffffff, 0xe3e3ffffffffffff, 0xc3c7ffffffffffff, 0x8387ffffffffffff,
    0x0303ffffffffffff, 0x2323ffffffffffff, 0xc1c1c1fffffffcfc, 0xc1c1c1fffffff8f8, 0xc1c1c1fffffff1f1, 0xc1c1c1ffffffe3e3,
    0xc1c1c1ffffffc7c7, 0xc1c1c1ffffff8f8f, 0xc1c1c1ffffff1f1f, 0xc1c1c1ffffff3f3f, 0xc1c1c1fffffcfcfc, 0xc1c1c1fffff8f8f8,
    0xc1c1c1fffff1f1f1, 0xc1c1c1ffffe3e3e3, 0xc1c1c1ffffc7c7c7, 0xc1c1c1ffff8f8f8f, 0xc1c1c1ffff1f1f1f, 0xc1c1c1ffff3f3f3f,
    0xc1c1c1fffcfcfcff, 0xc1c1c1fff8f8f8ff, 0xc1c1c1fff1f1f1ff, 0xc1c1c1ffe3e3e3ff, 0xc1c1c1ffc7c7c7ff, 0xc1c1c1ff8f8f8fff,
    0xc1c1c1ff1f1f1fff, 0xc1c1c1ff3f3f3fff, 0xc1c1c1fcfcfcffff, 0xc1c1c1f8f8f8ffff, 0xc1c1c1f1f1f1ffff, 0xc1c1c1e3e3e3ffff,
    0xc1c1c1c7c7c7ffff, 0xc1c1c18f8f8fffff, 0xc1c1c11f1f1fffff, 0xc1c1c13f3f3fffff, 0xc1c1c0fcfcffffff, 0xc1c1c0f8f8ffffff,
    0xc1c1c1f1f1ffffff, 0xc1c1c1e3e3ffffff, 0xc1c1c1c7c7ffffff, 0xc1c1818f8fffffff, 0xc1c1011f1fffffff, 0xc1c1013f3fffffff,
    0xc1c0c0fcffffffff, 0xe3e0e0f8ffffffff, 0xebe1f1f1ffffffff, 0xf7e3e3e3ffffffff, 0xebc3c7c7ffffffff, 0xe383838fffffffff,
    0xc101011fffffffff, 0xc101013fffffffff, 0xc0c0c0ffffffffff, 0xe0e0e0ffffffffff, 0xf1f1f1ffffffffff, 0x0000000000000000,
    0xc7c7c7ffffffffff, 0x838383ffffffffff, 0x010101ffffffffff, 0x010101ffffffffff, 0xc0c0c1ffffffffff, 0xe0e0e3ffffffffff,
    0xf1f1ffffffffffff, 0xe3e3ffffffffffff, 0xc7c7ffffffffffff, 0x8383e3ffffffffff, 0x0101c1ffffffffff, 0x0101c1ffffffffff,
};

template<Color C>
int evaluate_single_pawn(const Position& pos, int v)
{
    constexpr int FWD = C == WHITE ? 8 : -8;
    BitBoard own_pawns = pos.type_bb[PAWN] & pos.color_bb[C];

    if (popcount(pos.all_bb()) == 3)
    {
        unsigned flip = ((C == WHITE) ? 0 : 56) | ((own_pawns & QUEEN_SIDE) ? 0 : 7);
        unsigned opp_king = first_square(pos.type_bb[KING] & pos.color_bb[~C]) ^ flip;
        unsigned own_king = first_square(pos.type_bb[KING] & pos.color_bb[C]) ^ flip;
        unsigned pawn = first_square(own_pawns) ^ flip;

        unsigned idx = own_king + 64 * (C != pos.next) + 64 * 2 * (pawn % 8 + 4 * (pawn / 8 - 1));
        if (!(kpk_bitbase[idx] & (1ULL << opp_king)))
            return 0;
        return v + (C == pos.next ? 1 : -1) * material[QUEEN].end;
    }

    if (smear<FWD>(shift_signed<FWD>(own_pawns)) & pos.type_bb[KING] & pos.color_bb[~C])
        v /= 2;
    return v;
}

#ifndef TUNE
constexpr
#endif
Weight mobility_evals[66] =
{
    {-17, -73}, {-7, -28}, {-4, -3}, {-1, 3}, {0, 4}, {0, 7}, {0, 4}, {1, -1},
    {0, -5}, {-6, -110}, {-6, -41}, {-1, -31}, {-2, -12}, {0, -2}, {1, 6}, {4, 6},
    {2, 9}, {-2, 10}, {0, 3}, {2, -3}, {1, -8}, {0, -3}, {4, -13}, {-8, -182},
    {-4, -105}, {-1, -73}, {-1, -35}, {-2, -13}, {-1, -4}, {1, -1}, {4, 2}, {2, 12},
    {3, 14}, {5, 15}, {7, 14}, {13, 10}, {23, 3}, {27, -2}, {-8, -71}, {0, -71},
    {0, -66}, {0, -64}, {0, -45}, {0, -35}, {0, -28}, {0, -21}, {-1, -11}, {-1, -6},
    {-1, 0}, {0, 6}, {-2, 12}, {0, 16}, {-3, 20}, {-2, 21}, {-1, 21}, {-2, 22},
    {-5, 25}, {-3, 23}, {0, 18}, {2, 17}, {12, 7}, {6, 11}, {23, -2}, {13, 0},
    {21, -15}, {-18, 9},
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

    Score r{};

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
Weight king_evals[64][4] =
{
    {{-60, 4}, {-22, 7}, {7, -1}, {28, -10}},
    {{-2, -27}, {68, 97}, {74, 127}, {48, 48}},
    {{-1, 0}, {0, -1}, {2, -2}, {1, 0}},
    {{33, 3}, {-2, 29}, {-32, 57}, {-27, 23}},
    {{1, 3}, {-1, 5}, {8, -3}, {13, -29}},
    {{10, 1}, {7, 15}, {-18, 19}, {-151, -47}},
    {{-1, 0}, {1, -1}, {7, -7}, {7, -1}},
    {{19, 0}, {-5, 12}, {-93, 35}, {-42, -35}},
    {{-20, -2}, {-6, -1}, {10, -7}, {33, -5}},
    {{0, 15}, {28, 122}, {67, 97}, {-24, -3}},
    {{-5, -4}, {-11, -7}, {-3, -17}, {0, -20}},
    {{-10, -11}, {-38, 5}, {-65, 19}, {-24, -7}},
    {{-3, 0}, {-4, -5}, {-3, -14}, {0, -29}},
    {{-1, 12}, {4, 31}, {1, 47}, {86, -15}},
    {{-4, -5}, {-2, -10}, {-2, -11}, {-3, -14}},
    {{-1, -9}, {-14, 1}, {-78, 30}, {-78, -12}},
    {{13, -32}, {-14, -1}, {-6, 2}, {9, 3}},
    {{-1, -1}, {0, 43}, {-18, 98}, {59, 96}},
    {{2, 0}, {3, 8}, {0, 12}, {5, 9}},
    {{32, -3}, {-2, 1}, {-36, 5}, {-12, -26}},
    {{0, 0}, {-5, -3}, {-30, 13}, {-8, -13}},
    {{-1, -1}, {-23, 21}, {-7, 34}, {-17, -10}},
    {{-3, -1}, {0, -1}, {-7, 1}, {-5, 2}},
    {{13, -4}, {-21, 14}, {-81, 33}, {-38, -3}},
    {{5, -6}, {-10, 7}, {2, 5}, {29, -6}},
    {{-6, -1}, {-10, 50}, {-15, 87}, {53, 82}},
    {{4, -10}, {-8, 1}, {-1, 1}, {11, -2}},
    {{23, 0}, {-2, 2}, {-31, 4}, {-72, 9}},
    {{8, -1}, {-2, -2}, {-16, -2}, {-10, -28}},
    {{-3, -1}, {-30, 30}, {-18, 58}, {2, 4}},
    {{3, 0}, {-5, 3}, {-13, -1}, {-15, -15}},
    {{7, -13}, {-8, 0}, {-52, 24}, {-58, 25}},
    {{-38, -58}, {-16, 15}, {27, 55}, {60, 101}},
    {{-10, -5}, {6, 25}, {-2, 52}, {54, 58}},
    {{0, -7}, {-18, 44}, {17, 67}, {55, 68}},
    {{-3, -1}, {6, 6}, {-33, 19}, {-20, -8}},
    {{-13, -2}, {13, -1}, {16, 1}, {-21, 11}},
    {{1, -1}, {-15, 13}, {-13, 35}, {10, 45}},
    {{-15, 6}, {24, -8}, {34, -10}, {2, 10}},
    {{32, 0}, {7, 0}, {-25, 0}, {-29, -10}},
    {{-20, -22}, {-19, 45}, {13, 91}, {79, 110}},
    {{-8, 0}, {9, 25}, {6, 52}, {24, 65}},
    {{2, -9}, {-2, 23}, {0, 53}, {15, 78}},
    {{-1, -4}, {8, 0}, {-6, 12}, {-3, 11}},
    {{-12, -1}, {18, -8}, {33, -17}, {20, 15}},
    {{5, -1}, {-35, 32}, {12, 53}, {49, 35}},
    {{-6, 5}, {16, -6}, {15, -1}, {4, 10}},
    {{18, -8}, {-5, 2}, {-4, 5}, {-17, 20}},
    {{-47, -86}, {8, 26}, {80, 118}, {97, 78}},
    {{4, 4}, {2, 21}, {17, 42}, {59, 67}},
    {{-10, -44}, {3, 51}, {45, 79}, {68, 43}},
    {{14, 9}, {20, 33}, {13, 50}, {34, -28}},
    {{-1, -2}, {4, 6}, {2, 6}, {-66, 18}},
    {{9, -2}, {15, 38}, {17, 27}, {-16, -16}},
    {{-5, 8}, {13, 6}, {3, 2}, {46, 76}},
    {{35, 7}, {-18, 13}, {-8, -3}, {-20, -10}},
    {{-40, -79}, {19, 31}, {78, 111}, {243, 131}},
    {{9, 7}, {26, 16}, {10, 32}, {19, 66}},
    {{22, -32}, {36, 49}, {40, 78}, {55, 80}},
    {{-5, -13}, {11, 2}, {9, 26}, {32, 41}},
    {{0, 1}, {9, 4}, {20, -7}, {24, 14}},
    {{28, 0}, {19, 24}, {-14, 25}, {-114, -114}},
    {{24, 23}, {11, 24}, {17, 14}, {20, -22}},
    {{52, -1}, {23, 5}, {12, 3}, {-13, 10}},
};

#ifndef TUNE
constexpr
#endif
Weight piece_evals[10][24] =
{
    {
        {59, 63}, {46, 71}, {15, 0}, {16, 17}, {-2, -9}, {48, 13}, {-3, -7}, {31, -4},
        {-9, -39}, {-19, -31}, {58, 34}, {38, 36}, {3, 19}, {39, 52}, {-78, 0}, {-64, 9},
        {-67, 1}, {58, -12}, {34, 4}, {57, 69}, {-59, -14}, {24, -2}, {26, 32}, {54, 76},
    },
    {
        {69, 47}, {42, 54}, {8, 0}, {25, 4}, {-9, -4}, {46, 11}, {-2, -6}, {10, 1},
        {-14, -28}, {-31, -14}, {32, 37}, {50, 37}, {4, 18}, {39, 64}, {-82, 6}, {-40, 8},
        {0, -4}, {-5, -30}, {48, 39}, {23, 21}, {-10, -8}, {-49, -24}, {39, 59}, {16, 32},
    },
    {
        {52, 85}, {44, 70}, {17, 6}, {14, 21}, {-8, -5}, {30, 51}, {-1, -12}, {3, 31},
        {-7, -52}, {-5, -52}, {44, 69}, {10, 41}, {5, 22}, {42, 31}, {-39, -42}, {-38, -23},
        {-15, -39}, {1, 24}, {23, 11}, {27, 77}, {-26, -37}, {0, 8}, {22, 22}, {23, 76},
    },
    {
        {47, 75}, {44, 35}, {11, 10}, {18, 12}, {-8, -10}, {34, 34}, {-2, -10}, {1, 28},
        {-10, -34}, {-10, -42}, {33, 70}, {9, 56}, {-8, 31}, {24, 37}, {-4, -77}, {-53, 8},
        {-1, -10}, {-28, -31}, {35, 44}, {3, 15}, {-16, -8}, {-43, -51}, {20, 68}, {-15, 11},
    },
    {
        {50, 58}, {41, 61}, {28, -1}, {24, 4}, {-6, -1}, {54, 9}, {-2, -3}, {32, -1},
        {-15, -32}, {-28, -26}, {53, 23}, {4, 56}, {1, 27}, {53, 39}, {-90, 12}, {-42, 12},
        {-1, 0}, {-21, -13}, {48, 32}, {0, 35}, {-7, -1}, {-50, -15}, {32, 61}, {-16, 39},
    },
    {
        {42, 48}, {48, 40}, {26, 0}, {22, 2}, {-10, 0}, {33, 19}, {-5, 0}, {25, 0},
        {-7, -35}, {-28, -15}, {42, 17}, {22, 42}, {-10, 31}, {68, 27}, {-93, 15}, {-46, 17},
        {29, -1}, {-65, -42}, {63, 49}, {-36, -31}, {18, 2}, {-126, -26}, {56, 70}, {-43, -59},
    },
    {
        {36, 50}, {40, 79}, {34, 5}, {19, 3}, {1, -2}, {27, 23}, {7, 2}, {43, -3},
        {-25, -23}, {-7, -34}, {22, 31}, {25, 45}, {-22, 54}, {26, 37}, {-70, 9}, {-28, 0},
        {22, -1}, {-51, -24}, {63, 51}, {-34, -40}, {-1, 22}, {-47, -5}, {57, 82}, {25, 42},
    },
    {
        {39, 53}, {42, 69}, {34, 2}, {17, 1}, {0, 0}, {7, 31}, {3, -1}, {12, 11},
        {-8, -38}, {-2, -43}, {33, 14}, {33, 45}, {-14, 52}, {45, 21}, {-82, 15}, {-24, 5},
        {19, -3}, {-57, -58}, {35, 54}, {-8, -60}, {0, 15}, {-71, -51}, {63, 78}, {-23, 9},
    },
    {
        {40, 59}, {31, 54}, {23, 5}, {8, -1}, {13, 0}, {11, 28}, {1, 1}, {19, 9},
        {15, -42}, {-28, -40}, {14, 12}, {15, 44}, {12, 62}, {22, 59}, {-55, 2}, {-25, -22},
        {14, -18}, {-16, -24}, {24, 61}, {179, 73}, {3, 22}, {2, 0}, {52, 75}, {58, 112},
    },
    {
        {33, 48}, {27, 31}, {6, 2}, {-5, 0}, {7, 3}, {22, 31}, {10, 3}, {11, 15},
        {-19, -39}, {-19, -48}, {34, 19}, {35, 26}, {29, 54}, {30, 58}, {-55, 4}, {-29, -3},
        {14, -2}, {-2, -53}, {28, 46}, {69, 67}, {6, 27}, {-20, -8}, {58, 71}, {98, 13},
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
    Score r{};
    for (int i = 0; i < 8; i++)
    {
        r += king_evals[8 * eval_offset + i][std::min(3, v[i])];
    }

    constexpr int FWD = C == WHITE ? 8 : -8;

    BitBoard own_pawns = position.type_bb[PAWN] & position.color_bb[C];
    BitBoard own_attack = mobility.attacks[C][PAWN];

    BitBoard opp_pawns = position.type_bb[PAWN] & position.color_bb[~C];
    BitBoard opp_attack = mobility.attacks[~C][PAWN];

#ifndef TUNE
    const
#endif
    Weight* pe = piece_evals[eval_offset + 2 * (eval_offset >= 2 || popcount(position.all_bb()) > 16)];
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

        r += pe[4] * popcount(km & opp_attack & ~(position.color_bb[~C] & ~position.type_bb[PAWN]));
        r += pe[5] * popcount(km & position.color_bb[~C] & ~(position.type_bb[PAWN] | position.type_bb[KNIGHT]));
        r += pe[6] * popcount(km & position.type_bb[PAWN] & (position.color_bb[C] | opp_attack));

        constexpr BitBoard OUTPOST_RANKS = static_cast<BitBoard>(C == WHITE ? 0x00ffffff00000000ULL : 0x00000000ffffff00ULL);
        r += pe[7] * popcount((knights | (km & ~position.color_bb[C])) & own_attack & ~opp_attack & OUTPOST_RANKS);
    }

    BitBoard bishops = position.type_bb[BISHOP] & position.color_bb[C];
    if (position.type_bb[BISHOP])
    {
        BitBoard bblock = shift_signed<FWD - 1>(bishops & KING_SIDE) | shift_signed<FWD + 1>(bishops & QUEEN_SIDE);
        r += pe[8] * popcount(bblock & own_pawns);
        BitBoard bblock2 = shift_signed<2 * FWD - 2>(bishops & KING_SIDE) | shift_signed<2 * FWD + 2>(bishops & QUEEN_SIDE);
        r += pe[9] * popcount(bblock2 & (own_pawns | (opp_pawns & opp_attack)));
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
    r += pe[14] * popcount(safe_checks);
    r += pe[15] * popcount(king_attack[king_sq] & mobility.attacks2[~C] & ~guarded);
    r += pe[16 + !!(position.type_bb[QUEEN] & position.color_bb[C]) + 2 * !!((bishops & LIGHT_SQUARES) && (bishops & DARK_SQUARES)) + 4 * !!passed_pawns];

    return r;
}

alignas(64)
#ifndef TUNE
constexpr
#endif
Weight king_relative_pawn_psqt[32][48] =
{
    {
        {21, -21}, {13, 5}, {-12, 16}, {-28, 13}, {-31, 2}, {-11, -19}, {-15, -9}, {-3, -12},
        {31, -9}, {8, -15}, {-9, -9}, {-11, -16}, {-5, -19}, {-9, -33}, {-15, -15}, {-7, -21},
        {10, -23}, {6, -28}, {0, -22}, {8, -26}, {10, -31}, {0, -34}, {-4, -32}, {-5, -20},
        {8, 3}, {-7, -12}, {5, -10}, {21, -34}, {22, -39}, {1, -20}, {-5, -24}, {0, -6},
        {15, 31}, {16, 6}, {21, -11}, {22, -15}, {4, -8}, {10, -30}, {1, -8}, {-6, 1},
        {49, 63}, {38, 54}, {28, 39}, {33, 14}, {-3, 29}, {0, 22}, {30, 17}, {10, 30},
    },
    {
        {0, -17}, {30, 0}, {0, 1}, {-9, 5}, {-14, -10}, {-20, -14}, {-12, -2}, {-11, -4},
        {11, -11}, {43, -13}, {-10, 3}, {-6, -7}, {-15, -7}, {-14, -19}, {-15, -6}, {-10, -10},
        {2, 0}, {22, -11}, {1, -9}, {1, 0}, {0, -19}, {0, -28}, {-2, -23}, {-11, -10},
        {-1, 17}, {1, 10}, {6, 1}, {1, -8}, {11, -27}, {2, -15}, {1, -26}, {1, -1},
        {-1, 47}, {-15, 16}, {0, 3}, {-12, 3}, {-5, -22}, {-7, -16}, {-10, -12}, {-1, 0},
        {3, 57}, {-2, 44}, {16, 32}, {6, 15}, {3, 5}, {6, 5}, {5, 27}, {0, 25},
    },
    {
        {0, 2}, {19, -6}, {34, -1}, {-7, 23}, {-7, 4}, {-7, 5}, {-1, 3}, {-7, 6},
        {-3, 0}, {27, 0}, {25, -1}, {5, 3}, {-4, -1}, {-1, -6}, {-3, 2}, {-9, -6},
        {0, -5}, {18, -9}, {34, -16}, {28, -12}, {17, -22}, {12, -19}, {2, -16}, {-9, -10},
        {10, 10}, {9, 1}, {31, -5}, {25, -21}, {29, -36}, {14, -20}, {1, -9}, {-4, 1},
        {0, 25}, {-22, 20}, {1, -5}, {12, -16}, {-6, -12}, {0, -17}, {-8, -3}, {0, 4},
        {45, 27}, {23, 28}, {9, 34}, {16, 31}, {24, 16}, {17, 16}, {13, 13}, {27, 5},
    },
    {
        {-3, -3}, {17, -4}, {3, -7}, {10, 26}, {-8, 16}, {-8, 7}, {1, 17}, {-9, 2},
        {-15, -3}, {8, 1}, {1, 20}, {23, 3}, {0, 3}, {-5, 2}, {-3, 2}, {-6, -10},
        {-16, 5}, {-1, 2}, {18, -2}, {27, -12}, {18, -5}, {4, -6}, {-1, -12}, {-15, -12},
        {-11, 20}, {0, 3}, {-4, 17}, {14, -12}, {6, -12}, {-2, -2}, {-1, -9}, {-10, 14},
        {-6, 15}, {-37, 17}, {-50, 23}, {-51, -7}, {-26, 2}, {-36, 3}, {-25, 4}, {-7, 13},
        {-10, 28}, {-26, 37}, {-25, 31}, {-7, 28}, {-5, 13}, {-22, 15}, {-41, 35}, {-7, 17},
    },
    {
        {0, 0}, {26, 7}, {4, 5}, {-16, 10}, {-32, -2}, {-18, -14}, {-19, 4}, {-10, -6},
        {0, -12}, {4, -14}, {-6, -3}, {-11, -3}, {-7, -13}, {-13, -20}, {-16, -5}, {-13, -5},
        {20, -3}, {8, 1}, {5, -4}, {11, -8}, {7, -22}, {1, -32}, {-4, -21}, {-11, -16},
        {2, 35}, {-8, 25}, {3, 11}, {25, -18}, {24, -28}, {3, -14}, {-10, -14}, {-1, -3},
        {9, 58}, {19, 30}, {33, 0}, {25, -6}, {9, -21}, {6, -16}, {2, -9}, {2, -7},
        {51, 92}, {52, 59}, {41, 55}, {18, 18}, {7, 14}, {32, 5}, {10, 33}, {38, 8},
    },
    {
        {-23, -5}, {0, 0}, {9, 8}, {-14, 18}, {-25, -1}, {-27, 0}, {-8, -1}, {-27, 20},
        {-23, -5}, {19, -9}, {-9, 2}, {-11, 3}, {-14, -2}, {-16, -14}, {-15, 2}, {-19, 0},
        {5, -7}, {23, 5}, {4, 8}, {14, -5}, {13, -20}, {-3, -20}, {-9, -14}, {-13, -3},
        {5, 21}, {-3, 26}, {6, 22}, {15, -5}, {22, -22}, {-6, -6}, {-5, -10}, {-8, 0},
        {-9, 46}, {4, 53}, {-1, 31}, {15, -8}, {-2, -12}, {-3, -16}, {-11, -10}, {-12, 7},
        {58, 40}, {37, 93}, {28, 56}, {12, 26}, {23, -1}, {23, -2}, {10, 3}, {20, 11},
    },
    {
        {1, 1}, {12, 4}, {0, 0}, {-8, 24}, {-9, 9}, {-13, 6}, {-7, 17}, {-5, 3},
        {-6, -2}, {10, -5}, {18, -1}, {7, -3}, {0, 0}, {-1, -3}, {-4, 8}, {-9, 7},
        {2, 5}, {15, 12}, {34, 2}, {29, 7}, {19, -5}, {6, -5}, {0, -2}, {-2, -10},
        {2, 21}, {7, 21}, {25, 15}, {30, 3}, {25, -13}, {7, 0}, {6, -7}, {5, 7},
        {12, 17}, {8, 25}, {2, 9}, {17, -6}, {20, -19}, {-5, -13}, {-4, -14}, {12, -2},
        {23, 32}, {36, 39}, {48, 49}, {30, 12}, {21, 3}, {3, 5}, {31, 7}, {37, -9},
    },
    {
        {-14, 11}, {0, 15}, {6, 3}, {0, 0}, {1, 10}, {-10, 3}, {-7, 16}, {-23, 18},
        {-17, 11}, {1, 9}, {-4, 1}, {24, -6}, {-6, -5}, {-7, 5}, {-2, 12}, {-13, -2},
        {-14, 6}, {1, 14}, {18, 10}, {35, 2}, {17, 4}, {-5, 6}, {1, 1}, {-13, 7},
        {-5, 23}, {-3, 17}, {13, 23}, {16, 18}, {1, 12}, {4, 5}, {-3, 1}, {4, 6},
        {0, 11}, {-1, 12}, {-2, 7}, {1, -6}, {-9, -2}, {8, -15}, {-9, -1}, {8, 7},
        {26, 24}, {8, 38}, {1, 24}, {14, 19}, {7, 6}, {31, 10}, {2, 13}, {32, 16},
    },
    {
        {-27, -5}, {1, 4}, {-11, 0}, {0, -5}, {-26, -8}, {-21, -15}, {-27, -1}, {-13, -6},
        {0, 0}, {31, 1}, {-1, 0}, {-7, -5}, {12, -17}, {9, -31}, {-16, -9}, {-10, -21},
        {-8, -6}, {-4, -4}, {2, 3}, {0, 2}, {16, -20}, {11, -37}, {-16, -29}, {-5, -21},
        {9, 22}, {15, 15}, {18, 7}, {22, 0}, {14, -23}, {-16, -13}, {-26, -14}, {-17, -11},
        {49, 13}, {26, 33}, {36, 2}, {11, -15}, {-18, -16}, {-13, -35}, {-6, -32}, {-1, -25},
        {48, 59}, {52, 73}, {33, 42}, {18, 37}, {6, -10}, {5, 6}, {19, -12}, {15, -2},
    },
    {
        {-21, -8}, {-1, -2}, {2, -6}, {-10, -2}, {-23, 4}, {-16, 0}, {-5, 8}, {-7, -2},
        {-13, -1}, {0, 0}, {11, -1}, {6, -9}, {-7, -7}, {-7, -7}, {0, -6}, {-6, -6},
        {-3, 3}, {26, -5}, {3, -2}, {9, 2}, {-12, 5}, {-4, -11}, {-17, -7}, {-23, -1},
        {35, 5}, {19, 16}, {19, 19}, {40, -10}, {18, -6}, {-2, -8}, {-3, -8}, {9, -6},
        {52, 20}, {29, 36}, {61, 13}, {23, 5}, {15, -24}, {1, -23}, {1, -21}, {-2, -15},
        {44, 39}, {36, 74}, {49, 53}, {34, 9}, {13, -3}, {-1, -4}, {24, -3}, {44, -17},
    },
    {
        {-15, -1}, {-11, 7}, {-3, 1}, {4, 3}, {6, -13}, {-8, 1}, {-3, 12}, {-5, 9},
        {-16, 4}, {0, 4}, {0, 0}, {23, -2}, {9, -14}, {-4, -7}, {3, 2}, {-2, -3},
        {-12, 5}, {6, 3}, {25, 2}, {7, 10}, {12, -3}, {-4, 1}, {1, -9}, {-4, 2},
        {15, 8}, {15, 14}, {17, 23}, {16, 12}, {18, -5}, {3, 2}, {-3, -4}, {1, 0},
        {32, 12}, {49, 13}, {22, 24}, {29, 8}, {0, -2}, {12, -19}, {-4, -16}, {19, -13},
        {49, 15}, {35, 52}, {49, 45}, {27, 19}, {20, -2}, {4, -1}, {21, -8}, {37, 6},
    },
    {
        {-19, 15}, {1, -1}, {-9, 1}, {3, -1}, {-13, -2}, {-11, 0}, {-2, 11}, {-11, 13},
        {-10, 3}, {-7, 7}, {9, -1}, {0, 0}, {20, -4}, {-4, -7}, {-4, 7}, {-12, 12},
        {-1, 8}, {-1, 4}, {5, 4}, {23, 11}, {11, 4}, {6, -4}, {8, 4}, {-8, 0},
        {23, 4}, {7, 9}, {14, 19}, {40, 0}, {21, 8}, {12, 2}, {19, -5}, {14, 6},
        {33, 0}, {24, 6}, {13, 23}, {28, 11}, {20, 1}, {13, 0}, {11, -5}, {8, 1},
        {46, 16}, {27, 12}, {35, 25}, {35, 18}, {30, 3}, {24, -5}, {30, 2}, {28, -8},
    },
    {
        {-42, 9}, {-16, 26}, {-15, 0}, {-5, 5}, {-14, -10}, {-20, -9}, {-3, 1}, {-27, 4},
        {-7, 8}, {-3, 6}, {-20, 8}, {-28, 21}, {2, -8}, {-12, -15}, {-3, -8}, {-14, -6},
        {0, 0}, {3, 22}, {-2, 17}, {-1, 4}, {-1, -6}, {3, -23}, {-36, -8}, {-8, -13},
        {9, 25}, {10, 18}, {15, 45}, {10, 23}, {-14, 1}, {-9, 3}, {-18, -3}, {-27, 4},
        {42, 27}, {46, 40}, {38, 22}, {25, 3}, {-21, -27}, {-25, -31}, {1, -32}, {-8, -22},
        {66, 83}, {61, 65}, {40, 67}, {22, 13}, {-8, 18}, {2, -3}, {-1, -2}, {20, -18},
    },
    {
        {-33, 13}, {-14, 10}, {-11, 7}, {4, -8}, {-4, -3}, {-14, 10}, {-5, 18}, {-18, 18},
        {-10, 1}, {-8, 22}, {-1, 4}, {-20, 14}, {-2, 2}, {1, -3}, {10, 2}, {-20, 8},
        {-2, 25}, {0, 0}, {-1, 23}, {-4, 23}, {-17, 12}, {-4, 2}, {-18, -1}, {-13, 2},
        {36, 24}, {21, 29}, {28, 31}, {-4, 37}, {10, 8}, {-8, 7}, {8, -4}, {6, -1},
        {33, 38}, {64, 39}, {47, 42}, {15, 14}, {-16, 3}, {16, -19}, {-16, -13}, {-10, -6},
        {60, 52}, {78, 86}, {71, 55}, {27, 28}, {25, -12}, {30, 9}, {20, -2}, {11, 3},
    },
    {
        {-41, 22}, {-21, 20}, {-27, 12}, {-3, -1}, {-22, 4}, {0, -1}, {-26, 28}, {-23, 23},
        {-33, 19}, {-12, 12}, {-5, 14}, {7, 3}, {-9, 5}, {3, 0}, {9, 5}, {12, 5},
        {8, 16}, {-4, 18}, {0, 0}, {8, 28}, {15, 11}, {-21, 15}, {0, 4}, {-7, 2},
        {8, 29}, {17, 15}, {31, 39}, {9, 32}, {33, 13}, {16, 8}, {-4, 6}, {6, 14},
        {63, 13}, {35, 38}, {38, 44}, {46, 26}, {3, 9}, {-6, -1}, {-2, -11}, {11, -4},
        {48, 39}, {62, 51}, {44, 73}, {43, 41}, {13, 12}, {-5, -3}, {7, 8}, {15, -16},
    },
    {
        {-17, 19}, {-23, 15}, {-11, 4}, {8, -11}, {-50, 17}, {-20, 0}, {0, 13}, {-32, 36},
        {-8, 21}, {-10, 12}, {-2, 5}, {-3, 14}, {-26, 13}, {0, -1}, {-10, 16}, {-1, 8},
        {-25, 25}, {18, 11}, {18, 11}, {0, 0}, {0, 29}, {0, 12}, {0, 9}, {-5, 16},
        {17, 18}, {16, 18}, {8, 30}, {24, 33}, {0, 23}, {-5, 30}, {17, 8}, {6, 11},
        {2, 19}, {22, 11}, {44, 32}, {29, 36}, {29, 37}, {20, 7}, {0, 9}, {21, -3},
        {42, 4}, {35, 38}, {22, 43}, {41, 56}, {27, 31}, {13, 24}, {22, -4}, {28, 12},
    },
    {
        {-2, 39}, {-15, 27}, {-2, 2}, {-5, -5}, {-15, -18}, {-17, -10}, {-17, 11}, {-19, 11},
        {-10, -1}, {-12, -4}, {-48, -28}, {-14, 8}, {-10, -1}, {-14, -12}, {-9, -5}, {-12, 1},
        {-49, -1}, {-29, -5}, {-14, 1}, {12, 9}, {12, 0}, {-14, -8}, {-32, -3}, {-12, -11},
        {0, 0}, {7, 11}, {23, 16}, {23, 19}, {1, 4}, {-17, 5}, {-19, -11}, {0, -4},
        {-6, 7}, {17, -13}, {31, 30}, {42, -13}, {-5, -23}, {-5, -25}, {-4, -24}, {1, -17},
        {48, 37}, {44, 71}, {19, 41}, {36, 28}, {21, 5}, {3, -12}, {6, -5}, {-4, -1},
    },
    {
        {-16, 35}, {0, 30}, {-22, 21}, {-18, 12}, {-2, -7}, {-11, 15}, {-3, 24}, {-14, 23},
        {-36, -2}, {-34, 1}, {-28, -7}, {-19, -39}, {5, -2}, {-15, -2}, {-22, 10}, {-13, 5},
        {-41, -2}, {-31, 6}, {-32, -6}, {-28, -7}, {5, 9}, {-12, 12}, {-13, -2}, {-24, 0},
        {16, 6}, {0, 0}, {20, 25}, {-10, 5}, {-2, 16}, {-2, 9}, {0, 6}, {1, 0},
        {4, -3}, {15, 20}, {26, 7}, {17, 24}, {22, 6}, {-17, -9}, {-24, -20}, {-4, -14},
        {50, 49}, {38, 39}, {50, 69}, {16, 6}, {3, 1}, {18, -20}, {-7, -11}, {-7, -15},
    },
    {
        {-22, 26}, {-31, 31}, {-28, 36}, {-7, 9}, {-23, 16}, {-22, 12}, {1, 22}, {-15, 24},
        {-44, 0}, {-40, 7}, {-18, -1}, {-19, -13}, {-23, -30}, {-2, -4}, {1, 6}, {3, 4},
        {-22, 11}, {-44, -10}, {-6, -4}, {-4, -11}, {-22, -3}, {-2, 10}, {-16, 2}, {-5, 2},
        {-5, 19}, {-11, 7}, {0, 0}, {-10, 20}, {-3, -1}, {11, 20}, {-7, 15}, {0, 7},
        {21, 26}, {3, -3}, {23, 37}, {0, 13}, {5, 27}, {6, 7}, {7, -7}, {-8, -11},
        {22, 8}, {49, 62}, {42, 60}, {34, 61}, {12, 20}, {20, 14}, {4, 3}, {6, -2},
    },
    {
        {-16, 28}, {-18, 20}, {-28, 19}, {-13, 21}, {-6, 7}, {-26, 12}, {-9, 21}, {-20, 33},
        {-21, 22}, {-32, -13}, {-28, -5}, {-47, 1}, {-20, -15}, {-36, -22}, {-16, 12}, {-16, 20},
        {-5, 23}, {-25, 2}, {-33, -15}, {-43, 8}, {-26, -20}, {-16, -5}, {-8, 12}, {-7, 14},
        {10, 26}, {-3, 12}, {0, 7}, {0, 0}, {-12, 4}, {-22, 18}, {5, 26}, {3, 16},
        {39, 12}, {13, 32}, {9, 4}, {29, 26}, {-16, 4}, {20, 21}, {11, 6}, {-8, 5},
        {44, 22}, {0, 10}, {41, 62}, {52, 48}, {25, 47}, {10, 21}, {13, 35}, {15, -8},
    },
    {
        {2, 46}, {4, 21}, {0, 7}, {1, 11}, {-5, -10}, {0, 2}, {-7, 16}, {-8, 6},
        {7, 41}, {-3, 33}, {-7, 7}, {-6, -3}, {6, -9}, {-9, -10}, {-12, -1}, {12, -6},
        {-7, 25}, {7, 27}, {-1, -13}, {28, 1}, {3, 15}, {-10, -1}, {-6, -21}, {-10, -6},
        {-27, 29}, {4, 33}, {28, 40}, {20, 33}, {-8, 7}, {0, 1}, {-24, 4}, {-16, 2},
        {0, 0}, {25, 61}, {33, 53}, {13, 1}, {-23, -20}, {-25, -21}, {11, -30}, {-3, -5},
        {52, 52}, {45, 64}, {57, 94}, {30, 37}, {-6, -10}, {17, -19}, {-11, 1}, {7, -11},
    },
    {
        {12, 32}, {6, 38}, {-1, 24}, {3, 16}, {19, -30}, {2, 4}, {-5, 30}, {15, 13},
        {-5, 24}, {0, 53}, {1, 24}, {-2, 13}, {1, -6}, {3, -9}, {10, -7}, {-19, 16},
        {-19, 30}, {-12, 29}, {-16, 31}, {-6, -17}, {2, 20}, {5, 2}, {-13, 2}, {15, -13},
        {19, 33}, {18, 55}, {19, 40}, {2, 25}, {5, 16}, {22, 6}, {-3, -7}, {-3, 7},
        {44, 58}, {0, 0}, {27, 82}, {24, 27}, {6, 19}, {11, -26}, {-26, -26}, {-11, -30},
        {41, 47}, {48, 69}, {56, 75}, {46, 61}, {28, 7}, {-17, -14}, {-13, -11}, {-3, -7},
    },
    {
        {7, 20}, {-1, 26}, {0, 27}, {15, 7}, {-1, 5}, {-5, 5}, {-6, 24}, {12, 19},
        {-19, 28}, {6, 16}, {-5, 36}, {16, 2}, {-2, 2}, {9, -6}, {-20, 15}, {33, -7},
        {-5, 9}, {5, 24}, {13, 31}, {-2, 21}, {-21, 4}, {3, 10}, {1, -1}, {2, 3},
        {32, 41}, {-3, 29}, {32, 65}, {-4, 35}, {16, 9}, {17, 27}, {-1, 18}, {-4, -1},
        {29, 40}, {37, 56}, {0, 0}, {45, 83}, {16, 17}, {8, 16}, {-11, -17}, {-6, -1},
        {46, 81}, {40, 44}, {56, 72}, {37, 57}, {50, 66}, {-4, 0}, {-16, 18}, {-8, -9},
    },
    {
        {-12, 30}, {-24, 24}, {10, 12}, {10, 12}, {-6, -5}, {-12, 15}, {13, 17}, {3, 22},
        {-7, 20}, {5, 13}, {0, 7}, {-15, 25}, {-4, 3}, {-2, 0}, {14, 0}, {-1, 11},
        {0, 35}, {-3, -4}, {3, 16}, {24, 24}, {-10, 21}, {-21, -7}, {10, 6}, {0, 12},
        {20, 26}, {-14, 35}, {12, 30}, {7, 66}, {-11, 19}, {13, 26}, {27, 22}, {25, 14},
        {22, 20}, {0, 29}, {27, 57}, {0, 0}, {37, 65}, {32, 23}, {20, 24}, {15, -6},
        {27, 14}, {39, 73}, {12, 50}, {50, 65}, {22, 49}, {42, 60}, {21, 13}, {5, -8},
    },
    {
        {16, 35}, {12, 8}, {-21, 13}, {-20, 17}, {-25, -18}, {-3, 8}, {13, 14}, {25, -4},
        {16, 41}, {1, 20}, {15, 15}, {-7, 4}, {-3, 4}, {7, -8}, {-30, 2}, {1, -1},
        {41, 56}, {17, 31}, {21, 25}, {14, -8}, {-9, 4}, {23, -7}, {-1, -19}, {-11, -11},
        {-1, 35}, {4, 33}, {-16, -10}, {21, 19}, {-4, 1}, {-12, 0}, {-3, -13}, {-9, -7},
        {-36, -32}, {21, 27}, {16, 26}, {-4, -10}, {3, -21}, {-24, -25}, {-6, -15}, {-18, -28},
        {0, 0}, {50, 71}, {26, 49}, {44, 23}, {-25, -19}, {-28, -6}, {0, -3}, {-11, -11},
    },
    {
        {-14, 17}, {1, 44}, {11, 3}, {-24, 23}, {-2, -3}, {3, 3}, {9, 14}, {6, 8},
        {-7, 16}, {16, 32}, {0, 30}, {8, 9}, {11, -2}, {-16, 0}, {4, 3}, {28, -10},
        {18, 43}, {19, 60}, {-16, 43}, {8, 19}, {10, 1}, {15, 4}, {-3, -14}, {-9, -9},
        {10, 27}, {12, 43}, {5, 55}, {-22, -23}, {33, 6}, {10, -3}, {-1, -12}, {-4, -12},
        {39, 19}, {22, 31}, {13, 44}, {-4, 30}, {-3, -17}, {-4, -26}, {-38, -38}, {-9, -31},
        {37, 61}, {0, 0}, {45, 92}, {14, 33}, {2, 29}, {7, -31}, {-2, -20}, {-38, -20},
    },
    {
        {1, 16}, {7, 14}, {11, 18}, {-29, 12}, {-15, -9}, {-4, -4}, {10, 11}, {8, 9},
        {-2, 10}, {-8, 15}, {7, 19}, {-15, 17}, {8, -11}, {-13, -7}, {7, 0}, {-18, 11},
        {-6, 36}, {2, 20}, {25, 54}, {18, 10}, {5, 21}, {8, -12}, {-1, -7}, {-23, -3},
        {-14, -16}, {13, 21}, {9, 54}, {11, 32}, {-22, -35}, {5, 13}, {16, -1}, {10, -1},
        {-5, 10}, {0, 4}, {11, 40}, {20, 24}, {3, -6}, {-6, -5}, {10, -27}, {-19, -43},
        {1, 0}, {30, 52}, {0, 0}, {39, 79}, {-3, 6}, {17, 6}, {-14, -8}, {-8, -28},
    },
    {
        {-13, 18}, {-3, 5}, {-2, 7}, {8, 1}, {-7, -8}, {-3, -2}, {10, 5}, {-7, 14},
        {-24, 19}, {7, 6}, {-5, 3}, {-1, 18}, {-6, -3}, {4, -3}, {-9, 5}, {0, 0},
        {-10, 12}, {-10, 18}, {-1, 31}, {27, 43}, {13, 31}, {-10, 13}, {-3, -3}, {-14, 13},
        {2, 35}, {-17, -28}, {3, 23}, {-1, 46}, {-11, 18}, {-29, -14}, {13, 24}, {-19, 8},
        {12, 1}, {-3, 7}, {28, 5}, {18, 33}, {-12, 10}, {4, -10}, {4, -2}, {3, -20},
        {13, 18}, {-6, -12}, {56, 54}, {0, 0}, {42, 55}, {6, 6}, {37, 24}, {-17, -24},
    },
    {
        {-3, 28}, {26, -31}, {-7, -12}, {-32, 0}, {-36, -11}, {27, 1}, {15, 3}, {-17, 2},
        {10, 22}, {26, -25}, {25, -2}, {-9, -7}, {6, -2}, {15, -19}, {5, -22}, {11, -34},
        {12, 48}, {-6, 13}, {-15, -12}, {-16, -9}, {4, 9}, {-13, -18}, {-8, -39}, {3, -18},
        {35, 61}, {10, -3}, {22, 30}, {23, 30}, {-6, 8}, {20, 0}, {21, 6}, {-19, -24},
        {-15, -2}, {45, 62}, {14, 14}, {-6, -7}, {-33, -19}, {8, 4}, {-7, -28}, {-22, -10},
        {-44, -71}, {52, 72}, {48, 49}, {35, 24}, {22, 71}, {57, 10}, {36, 20}, {8, -12},
    },
    {
        {0, 26}, {0, 38}, {16, -7}, {24, -1}, {-5, 9}, {-2, -4}, {6, 18}, {17, 1},
        {-5, 9}, {4, 25}, {25, 9}, {1, 19}, {10, 12}, {9, -11}, {-8, 4}, {-4, 6},
        {12, 39}, {15, 45}, {20, 28}, {11, 13}, {3, 5}, {12, 3}, {6, -14}, {-15, -17},
        {14, 57}, {56, 80}, {29, 42}, {14, 14}, {12, 4}, {15, 3}, {8, -7}, {-11, -6},
        {9, 22}, {40, 66}, {29, 58}, {-7, -20}, {1, -14}, {-12, -7}, {6, -43}, {-8, -34},
        {6, 11}, {37, 53}, {30, 69}, {34, 30}, {32, 26}, {4, 34}, {-13, -12}, {-20, 2},
    },
    {
        {-10, -5}, {2, -4}, {-16, 1}, {-33, -19}, {59, 18}, {-18, -6}, {-4, 7}, {1, -6},
        {-1, -11}, {0, 1}, {27, 11}, {-10, 3}, {-14, -26}, {-15, -4}, {15, -10}, {11, -11},
        {1, 12}, {8, 7}, {19, 34}, {7, 23}, {9, 25}, {2, 2}, {-13, -15}, {-8, -18},
        {22, 31}, {30, 27}, {49, 75}, {14, 36}, {17, 24}, {6, 0}, {-2, 7}, {8, -22},
        {-9, -35}, {6, 22}, {49, 76}, {57, 46}, {-17, -44}, {30, 15}, {-12, -16}, {-15, -31},
        {0, -6}, {3, -13}, {29, 55}, {21, 33}, {-7, 15}, {20, 44}, {17, 21}, {-22, -25},
    },
    {
        {3, 2}, {-7, -5}, {14, -15}, {-20, -13}, {-20, 7}, {-3, -30}, {14, 11}, {-8, 6},
        {15, -2}, {-22, 11}, {9, -9}, {16, 3}, {1, -2}, {6, -5}, {-16, 2}, {-6, -11},
        {-11, 0}, {-4, -4}, {13, 22}, {11, 38}, {27, 21}, {19, -4}, {-8, -15}, {2, -13},
        {4, 8}, {24, 14}, {27, 40}, {40, 69}, {9, 39}, {-16, 27}, {9, 4}, {5, 8},
        {11, 5}, {-13, -31}, {23, 35}, {40, 87}, {24, 36}, {-23, -32}, {-5, 2}, {-6, -18},
        {30, 10}, {15, 8}, {18, -1}, {45, 43}, {23, -14}, {18, 14}, {19, 23}, {-9, -6},
    },
};

alignas(64)
#ifndef TUNE
constexpr
#endif
Weight king_relative_knight_psqt[16][64] =
{
    {
        {-25, -33}, {-1, -32}, {-5, -35}, {0, -17}, {-1, -12}, {-9, -36}, {-18, -65}, {-38, -24},
        {-4, 3}, {0, -18}, {-6, -18}, {-2, -13}, {-5, -12}, {-11, -25}, {0, -45}, {-30, -47},
        {1, -3}, {5, -6}, {-1, 19}, {9, 11}, {1, 7}, {2, -8}, {-6, -8}, {-20, -42},
        {0, 9}, {7, 16}, {2, 24}, {12, 14}, {13, 8}, {9, 0}, {6, -15}, {-3, -27},
        {4, 3}, {15, 22}, {13, 14}, {3, 15}, {16, 11}, {15, -13}, {14, -21}, {-6, -44},
        {-6, 5}, {28, -2}, {48, -14}, {43, 0}, {32, -6}, {5, -13}, {-4, -12}, {-34, -16},
        {-1, -10}, {2, -11}, {14, -7}, {34, -12}, {4, -5}, {13, -15}, {-4, -21}, {-17, -20},
        {-24, -39}, {-5, -19}, {-9, 9}, {10, 2}, {-7, -2}, {-12, -14}, {-26, -25}, {-23, -21},
    },
    {
        {-16, -35}, {-18, -18}, {-8, -12}, {-8, -11}, {-9, -6}, {-7, -23}, {0, -41}, {-44, -15},
        {-17, -7}, {-15, -13}, {-6, -14}, {-3, 1}, {2, -17}, {-6, -18}, {-18, -28}, {-13, -37},
        {-11, 8}, {7, 9}, {5, 10}, {-1, 22}, {-2, 12}, {4, -1}, {-3, 0}, {-5, -26},
        {-7, 5}, {5, 16}, {-1, 27}, {0, 24}, {0, 25}, {-1, 16}, {12, -14}, {-2, -21},
        {0, 4}, {1, 34}, {1, 20}, {-2, 24}, {7, 16}, {6, 7}, {7, 11}, {-4, -29},
        {-18, -6}, {19, -13}, {9, 7}, {16, 12}, {8, 19}, {2, -4}, {-4, 3}, {-10, -19},
        {-3, -8}, {-2, -2}, {0, 7}, {17, 2}, {20, -18}, {1, 4}, {-16, 0}, {8, -17},
        {-23, -28}, {-14, 2}, {-22, 6}, {2, 1}, {7, -6}, {-10, -6}, {-10, 3}, {-30, 5},
    },
    {
        {-22, 1}, {-13, 10}, {-3, 15}, {1, -3}, {-1, 1}, {-20, 15}, {-8, -33}, {-8, -45},
        {-9, -1}, {-8, 1}, {21, 4}, {2, 14}, {3, 3}, {1, -8}, {-3, -17}, {-11, -36},
        {-2, -1}, {1, 21}, {25, 8}, {9, 25}, {8, 23}, {4, 17}, {-1, 6}, {-11, -16},
        {5, 2}, {3, 19}, {13, 20}, {12, 16}, {11, 15}, {7, 15}, {17, -2}, {-6, -6},
        {-4, -1}, {18, 10}, {8, 16}, {16, 5}, {17, 4}, {1, 18}, {16, -1}, {-8, -5},
        {-12, -14}, {29, -11}, {12, -2}, {25, 1}, {18, 14}, {12, -3}, {15, -13}, {-11, -4},
        {7, -17}, {-3, 2}, {17, -6}, {8, 5}, {9, 3}, {9, 4}, {-3, -6}, {-18, 1},
        {-46, -40}, {-37, -8}, {-32, -14}, {1, -14}, {-2, 0}, {-18, -4}, {-18, 2}, {-51, -24},
    },
    {
        {5, -31}, {-2, -14}, {-2, 0}, {-1, 22}, {3, 21}, {-2, 1}, {0, -5}, {-7, -40},
        {-13, -26}, {1, -9}, {5, 7}, {4, 21}, {1, 11}, {0, -3}, {-5, -19}, {-16, -12},
        {-4, -8}, {3, 24}, {16, 12}, {14, 29}, {7, 33}, {7, 15}, {-4, 20}, {-2, -16},
        {3, -2}, {4, 12}, {6, 30}, {1, 25}, {3, 31}, {4, 24}, {-1, 8}, {-4, 5},
        {-4, -7}, {4, 19}, {-6, 34}, {-1, 26}, {1, 24}, {-10, 25}, {14, 10}, {-10, 3},
        {-18, 7}, {5, -2}, {-8, 8}, {9, 15}, {13, 9}, {-23, 23}, {7, 1}, {-7, -7},
        {-13, -12}, {-16, 1}, {11, 1}, {-15, 6}, {3, 0}, {20, 0}, {-12, -4}, {-13, 8},
        {-65, -27}, {-21, 3}, {-46, -7}, {-24, 1}, {-27, 6}, {-42, 2}, {-41, 13}, {-53, -29},
    },
    {
        {-38, -12}, {-34, -28}, {-31, -18}, {-18, -26}, {-25, -36}, {-21, -35}, {-64, -51}, {-36, -38},
        {-24, 0}, {-13, -5}, {-23, -24}, {-11, -9}, {-49, 3}, {-27, -26}, {-34, -39}, {-42, -35},
        {3, 12}, {14, 6}, {9, -9}, {2, 1}, {-7, -2}, {-24, -18}, {-22, -25}, {-36, -31},
        {-4, 12}, {17, -9}, {34, -6}, {24, -16}, {10, -11}, {-2, -14}, {-7, -31}, {-53, -39},
        {6, -3}, {16, 6}, {25, -4}, {15, 6}, {39, -25}, {-6, -22}, {-22, -19}, {-29, -34},
        {10, -1}, {11, 2}, {7, 1}, {23, 3}, {15, -4}, {-8, -23}, {-18, -26}, {-42, -41},
        {-5, -13}, {7, -11}, {1, -5}, {4, -4}, {0, -9}, {1, -15}, {-20, -23}, {-18, -15},
        {-6, -12}, {-18, 0}, {7, -8}, {11, -17}, {0, -13}, {-28, -7}, {-26, -27}, {-35, -16},
    },
    {
        {-8, 13}, {-24, -13}, {-3, -3}, {-13, -6}, {-7, -7}, {-20, -17}, {-50, -28}, {-37, -52},
        {-12, 3}, {-28, 21}, {6, -10}, {-21, 13}, {-19, -4}, {-22, -9}, {-33, -14}, {-15, -19},
        {-15, 28}, {7, 22}, {15, 5}, {11, 0}, {-3, 0}, {-16, -3}, {-19, -8}, {-21, -28},
        {-11, 25}, {11, 3}, {-7, 4}, {19, -7}, {25, -14}, {5, -4}, {-3, -14}, {-42, -15},
        {6, 5}, {28, 2}, {14, 0}, {29, -1}, {20, -11}, {31, -18}, {8, -21}, {-18, -22},
        {-7, 6}, {10, 10}, {16, 0}, {28, 11}, {25, -5}, {10, -14}, {-12, -14}, {-20, -22},
        {5, -1}, {-20, 11}, {-12, 9}, {27, -6}, {14, -15}, {6, -6}, {-11, -7}, {-15, -13},
        {-16, -11}, {5, 4}, {-2, 15}, {6, -1}, {-1, -4}, {-2, -8}, {-37, -4}, {-24, -17},
    },
    {
        {-13, 25}, {-17, 8}, {-8, 12}, {-12, 12}, {5, 16}, {-17, -8}, {-33, -10}, {-44, -25},
        {-4, 20}, {-6, 23}, {-13, 19}, {-6, 15}, {-23, 16}, {-13, -3}, {-16, -1}, {-13, -3},
        {-15, 21}, {-9, 30}, {15, 8}, {29, 1}, {7, 6}, {-9, 1}, {-3, -4}, {-8, -15},
        {6, 9}, {-12, 15}, {6, 4}, {1, 6}, {21, -4}, {26, -8}, {7, -15}, {-6, -1},
        {5, -1}, {15, -2}, {37, -6}, {44, -16}, {20, -8}, {32, -11}, {12, -9}, {-3, -19},
        {-6, 6}, {15, -1}, {9, 6}, {27, 8}, {20, 2}, {17, -8}, {-7, -6}, {1, -19},
        {-3, -2}, {-2, 8}, {-28, 12}, {14, 3}, {3, 5}, {-7, -2}, {-1, -2}, {0, -6},
        {-23, -17}, {-7, 1}, {12, 7}, {5, 0}, {-9, 3}, {-4, -3}, {-4, -13}, {-42, -16},
    },
    {
        {-4, 35}, {-9, 23}, {-24, 21}, {-1, 19}, {-19, 28}, {-4, 5}, {-27, 7}, {-16, -1},
        {-37, 29}, {-1, 10}, {-18, 21}, {-15, 29}, {-2, 18}, {-32, 11}, {-4, 0}, {-14, -16},
        {-22, 3}, {-4, 19}, {3, 15}, {5, 14}, {15, 12}, {11, 2}, {-9, 4}, {-26, -3},
        {-24, 15}, {7, 4}, {2, 7}, {6, 1}, {-1, 5}, {26, -5}, {21, -9}, {-21, 12},
        {-28, 6}, {9, 3}, {14, 2}, {14, 7}, {17, -6}, {30, -5}, {-2, 4}, {-19, -3},
        {3, -5}, {3, 5}, {7, -1}, {13, 16}, {33, 0}, {6, 0}, {10, -4}, {9, -8},
        {-3, 1}, {10, 0}, {-15, 22}, {6, 5}, {24, 5}, {-6, 8}, {0, 2}, {-1, -5},
        {-18, -23}, {10, 2}, {3, 7}, {9, 2}, {-15, 6}, {-2, 7}, {1, 0}, {-31, -12},
    },
    {
        {-106, -11}, {-19, -13}, {-8, -24}, {-21, -8}, {-6, -19}, {-18, -33}, {-19, -21}, {-45, -36},
        {-1, -20}, {-19, -5}, {-19, -21}, {-14, 3}, {-30, -29}, {-24, -42}, {-22, -25}, {-33, -19},
        {-22, -8}, {0, 1}, {-1, -9}, {8, -22}, {-5, -17}, {-21, -36}, {-40, -40}, {-18, -23},
        {-1, 5}, {-1, -8}, {0, -2}, {1, -12}, {0, -21}, {-18, -26}, {-15, -26}, {-28, -13},
        {-4, -4}, {13, -7}, {5, 2}, {14, -16}, {1, -21}, {-8, -20}, {-10, -29}, {-20, -29},
        {6, -2}, {10, -19}, {0, 7}, {31, -2}, {-6, -3}, {-9, -17}, {-11, -36}, {-3, -28},
        {0, 8}, {6, -16}, {24, -3}, {-6, 0}, {16, -19}, {-5, -15}, {-20, -21}, {-1, -5},
        {-16, 5}, {0, -8}, {-5, -12}, {-16, -7}, {-8, -13}, {-18, -6}, {-13, -33}, {39, -21},
    },
    {
        {-29, 4}, {-22, -28}, {5, -18}, {-1, -4}, {-2, 4}, {-5, -12}, {-15, 0}, {-11, 1},
        {-21, 4}, {-10, 16}, {-28, -20}, {-12, -2}, {-15, -15}, {-26, -34}, {-17, -7}, {-10, 4},
        {-1, -5}, {-6, 3}, {-11, -1}, {8, -6}, {13, -15}, {0, -21}, {-18, -17}, {-21, -35},
        {-1, 20}, {-5, -11}, {9, -3}, {7, -11}, {4, -25}, {-21, -8}, {-7, -24}, {-27, -18},
        {3, 14}, {-15, -11}, {-4, 2}, {20, -14}, {-8, -20}, {-6, -16}, {-14, -12}, {-8, -28},
        {-10, 9}, {10, 4}, {14, 3}, {-1, 19}, {29, -2}, {-5, -12}, {11, -21}, {-8, -23},
        {12, 12}, {7, 5}, {4, 9}, {21, -4}, {0, -9}, {-21, -10}, {17, -25}, {18, -3},
        {17, 20}, {21, 10}, {6, 19}, {15, -2}, {3, -4}, {-9, -22}, {-22, 0}, {-12, -11},
    },
    {
        {8, 19}, {-30, -7}, {-7, -7}, {-2, -19}, {-1, 3}, {-13, -19}, {-10, -2}, {-24, -20},
        {-23, 16}, {-6, 0}, {-7, -7}, {-8, 3}, {-11, -16}, {-28, -25}, {-23, -5}, {-14, -20},
        {-32, -1}, {-7, 7}, {-17, -5}, {-9, -5}, {2, -7}, {-26, -14}, {-21, -19}, {-32, -21},
        {-14, 3}, {6, 4}, {-2, -2}, {15, -12}, {0, -17}, {-9, -6}, {-12, -29}, {-10, -30},
        {-6, -3}, {-3, 0}, {23, -6}, {15, -10}, {0, -2}, {6, -25}, {-14, -12}, {-15, -24},
        {13, 7}, {5, 15}, {20, 4}, {26, 2}, {18, -3}, {7, -14}, {-24, -14}, {-36, -31},
        {12, 3}, {3, 21}, {-13, 19}, {15, 12}, {9, -7}, {-15, -1}, {-8, -35}, {-2, -9},
        {58, 28}, {18, 35}, {11, 15}, {20, 9}, {3, 0}, {-4, -11}, {-13, -20}, {-8, -3},
    },
    {
        {-3, -1}, {-23, -14}, {-9, -3}, {13, -4}, {-13, -26}, {-15, -18}, {-33, -10}, {-11, 8},
        {4, 3}, {-8, -12}, {-11, -12}, {-22, -1}, {-6, 0}, {-11, -17}, {-17, -9}, {-4, -16},
        {-4, 2}, {-17, 10}, {-20, -6}, {-11, 0}, {-2, -6}, {-18, -15}, {-13, -19}, {-5, -28},
        {-21, 15}, {-1, -6}, {13, -10}, {3, -16}, {8, -9}, {-4, -18}, {3, -21}, {-19, -4},
        {-12, -11}, {11, 5}, {14, -7}, {10, -11}, {1, -7}, {6, 1}, {-2, -8}, {-9, -10},
        {5, -6}, {3, 17}, {12, 5}, {11, 24}, {11, 8}, {16, 0}, {-7, -2}, {3, -16},
        {1, 4}, {-4, 19}, {-19, 18}, {11, 16}, {5, 14}, {14, 6}, {-21, -8}, {-11, -2},
        {3, 23}, {1, 3}, {25, 14}, {12, 10}, {-2, 2}, {4, 7}, {15, 1}, {8, 3},
    },
    {
        {-74, 14}, {-24, 1}, {28, -19}, {26, -13}, {62, -13}, {3, -8}, {-86, -50}, {-73, -10},
        {-181, -59}, {-65, -1}, {-17, -16}, {3, -10}, {-13, -18}, {-25, -39}, {-60, 0}, {-31, -4},
        {52, -37}, {-24, 10}, {-8, -4}, {-30, -15}, {15, -13}, {-31, -20}, {-15, -18}, {-49, -16},
        {-13, 0}, {4, 3}, {-14, -1}, {14, -21}, {-3, -9}, {-14, -32}, {-24, -35}, {-18, -29},
        {-26, -11}, {-7, -6}, {17, -12}, {-16, -8}, {-4, 3}, {-1, -15}, {-34, -16}, {-29, -43},
        {-5, -15}, {25, 10}, {-10, -6}, {-1, 0}, {32, -3}, {5, -27}, {-10, -59}, {-49, -16},
        {-10, 30}, {-16, -18}, {-1, -28}, {-4, -41}, {-26, -6}, {22, -6}, {23, -9}, {-157, -17},
        {25, -6}, {-1, -17}, {-36, 25}, {23, -9}, {25, -8}, {36, -36}, {-5, -3}, {71, -47},
    },
    {
        {-53, -58}, {-10, -37}, {-37, -28}, {4, -34}, {28, 2}, {-8, -12}, {-40, -10}, {-29, -14},
        {90, -1}, {-43, -13}, {-25, -24}, {-4, 4}, {-21, -23}, {-7, -22}, {-23, -16}, {0, -1},
        {-35, -24}, {-5, 2}, {11, 9}, {-8, -2}, {-7, -13}, {-23, -25}, {-13, -18}, {-48, -39},
        {-12, -7}, {-9, 6}, {7, -3}, {-3, -15}, {-11, -18}, {-1, -12}, {1, -14}, {0, -5},
        {0, 10}, {5, -21}, {-19, -1}, {1, -15}, {10, -23}, {-8, -16}, {-19, -19}, {-14, -34},
        {20, 14}, {5, -2}, {3, -1}, {-11, -29}, {-2, -3}, {11, -22}, {-26, -15}, {-7, -15},
        {13, 8}, {-37, -18}, {13, -20}, {11, -9}, {-17, -13}, {-8, -29}, {-4, 1}, {-3, 3},
        {21, 19}, {10, 2}, {-13, 4}, {-12, -15}, {-32, -18}, {34, -17}, {-73, 24}, {-41, -43},
    },
    {
        {31, -45}, {-29, -54}, {-38, -16}, {-17, -40}, {18, -15}, {-36, -5}, {-13, -28}, {-31, -32},
        {-37, -15}, {19, -24}, {2, -22}, {-3, -23}, {-1, -13}, {-27, -26}, {-27, -22}, {-5, -11},
        {-25, -19}, {-14, -9}, {-18, -17}, {7, -11}, {3, -25}, {-27, -49}, {-22, -29}, {-24, -2},
        {-16, 7}, {-2, -7}, {-16, -16}, {1, -22}, {-4, -18}, {-24, -28}, {-9, -19}, {-24, -29},
        {7, 17}, {-19, -7}, {15, -27}, {8, -7}, {-15, -10}, {-20, -22}, {2, -11}, {-1, -7},
        {26, 15}, {9, 0}, {-16, 2}, {0, -6}, {1, -2}, {-8, -13}, {-22, -7}, {-23, -47},
        {55, -1}, {7, -8}, {30, 11}, {0, -2}, {0, -26}, {-16, -13}, {-40, -3}, {-24, -40},
        {17, -9}, {-3, 10}, {20, 9}, {-9, -2}, {-12, -6}, {-13, -36}, {-6, -20}, {-29, -67},
    },
    {
        {58, 35}, {-40, -6}, {-14, -8}, {-64, -33}, {-21, -1}, {-4, -22}, {-24, -14}, {-19, -29},
        {-58, 1}, {-27, -21}, {-2, -27}, {-11, -5}, {-29, -24}, {-21, -23}, {3, -15}, {-32, -29},
        {-54, -4}, {-10, -15}, {-31, -21}, {-9, -30}, {7, -38}, {-17, -31}, {-16, -23}, {-46, -7},
        {-16, -6}, {-9, -14}, {-9, -20}, {1, -18}, {3, -26}, {-11, -19}, {-1, -15}, {-27, -19},
        {6, -31}, {7, -4}, {16, -18}, {-8, -20}, {-12, -2}, {-16, -2}, {6, -15}, {-13, -30},
        {-25, 4}, {5, 14}, {11, 0}, {9, 4}, {4, 10}, {20, -10}, {30, 13}, {-26, 28},
        {31, 4}, {-5, -1}, {10, 22}, {15, -7}, {5, 9}, {8, -12}, {20, 9}, {16, 0},
        {-31, 38}, {39, 16}, {10, 2}, {16, 3}, {20, 0}, {5, -4}, {12, 7}, {59, -16},
    },
};

alignas(64)
#ifndef TUNE
constexpr
#endif
Weight king_relative_opp_pawn_psqt[16][48] =
{
    {
        {46, 90}, {25, 86}, {12, -7}, {-38, -70}, {-48, -108}, {-48, -119}, {-59, -127}, {-49, -95},
        {-8, 51}, {-27, -3}, {-36, 29}, {-5, -37}, {0, -66}, {0, -85}, {7, -88}, {12, -68},
        {13, 6}, {-2, 6}, {-8, 8}, {-19, 6}, {-5, -19}, {5, -29}, {6, -40}, {16, -37},
        {15, 11}, {-3, 10}, {-13, 15}, {-5, 9}, {1, 5}, {0, -6}, {7, -8}, {3, -5},
        {15, 2}, {4, 0}, {2, -4}, {0, 1}, {3, 5}, {3, 0}, {-1, -2}, {5, 1},
        {8, 3}, {-1, -7}, {0, -18}, {3, -7}, {7, -12}, {-1, -13}, {0, -5}, {0, 4},
    },
    {
        {-23, 86}, {46, 75}, {-9, 53}, {-13, -20}, {-25, -86}, {-27, -123}, {-47, -129}, {-52, -104},
        {-11, 38}, {-42, 24}, {-38, 24}, {-3, -7}, {1, -48}, {4, -75}, {17, -103}, {13, -72},
        {-4, 23}, {3, 5}, {-21, 24}, {-21, 8}, {-8, -11}, {1, -30}, {0, -33}, {15, -43},
        {9, 10}, {3, 6}, {-5, 6}, {-7, 3}, {0, 4}, {-1, -1}, {5, -13}, {1, -6},
        {9, 11}, {5, 2}, {0, -3}, {-3, 4}, {0, 12}, {4, -1}, {-5, -1}, {3, 1},
        {6, 7}, {1, -6}, {-1, -19}, {-1, 1}, {2, -4}, {1, -12}, {-6, -3}, {0, 1},
    },
    {
        {-11, 40}, {2, 41}, {27, 86}, {28, 47}, {12, -10}, {-2, -81}, {-69, -98}, {-33, -140},
        {-13, 40}, {-30, 35}, {-63, 61}, {-33, 59}, {-10, 21}, {-6, -27}, {-5, -53}, {24, -70},
        {-2, 10}, {-21, 18}, {-13, 14}, {-20, 8}, {-8, -2}, {1, -11}, {-1, -20}, {15, -35},
        {3, 15}, {-5, 7}, {3, 8}, {-9, 2}, {-2, 6}, {0, -1}, {0, -3}, {5, -5},
        {11, 5}, {1, 5}, {7, 1}, {0, 11}, {0, 11}, {7, 0}, {0, 0}, {7, 2},
        {4, 2}, {-1, -1}, {4, -7}, {7, 3}, {2, 5}, {-2, -4}, {-1, 0}, {3, 1},
    },
    {
        {-15, -44}, {3, 1}, {1, 42}, {5, 73}, {-37, 42}, {14, -10}, {-16, -83}, {-31, -127},
        {-3, -4}, {-3, 5}, {-14, 35}, {-23, 45}, {-20, 58}, {-9, 16}, {3, -41}, {15, -46},
        {1, 3}, {-4, -2}, {-10, 13}, {-3, -2}, {-15, 10}, {3, -8}, {-1, -10}, {8, -16},
        {7, 8}, {2, 5}, {-5, 8}, {5, 5}, {0, -1}, {-1, 7}, {10, -6}, {13, -3},
        {5, 13}, {-7, 12}, {5, 5}, {10, 8}, {4, 12}, {6, 7}, {2, 7}, {13, 3},
        {8, -5}, {-2, 0}, {3, -6}, {16, -1}, {3, 10}, {0, 3}, {3, -1}, {9, -2},
    },
    {
        {17, 79}, {16, 41}, {-21, 0}, {-64, -64}, {-68, -106}, {-70, -98}, {-78, -80}, {-49, -72},
        {-3, 50}, {14, 78}, {2, 45}, {-3, -18}, {-25, -62}, {-8, -67}, {-30, -63}, {-35, -33},
        {28, 30}, {7, 35}, {-9, 32}, {-19, 10}, {-5, -20}, {16, -35}, {32, -45}, {19, -35},
        {-16, 18}, {-46, 14}, {2, 13}, {19, 3}, {12, 4}, {12, -3}, {34, -14}, {19, -6},
        {0, 1}, {-9, -1}, {-15, 0}, {13, 0}, {12, 9}, {16, 1}, {7, 10}, {18, 4},
        {-9, 0}, {-16, -8}, {-7, -18}, {-4, 6}, {-4, 16}, {2, 2}, {0, 9}, {9, 7},
    },
    {
        {42, 70}, {24, 56}, {23, 45}, {-19, -25}, {-51, -57}, {-74, -83}, {-75, -82}, {-78, -59},
        {-23, 96}, {-5, 52}, {1, 68}, {-5, 22}, {-8, -32}, {-4, -56}, {-24, -69}, {3, -56},
        {-1, 22}, {-19, 37}, {0, 31}, {-6, 20}, {-7, -5}, {28, -37}, {11, -35}, {19, -37},
        {0, 14}, {-31, 11}, {0, 12}, {0, 7}, {13, 1}, {3, -2}, {20, -12}, {13, -7},
        {-15, 3}, {-1, -3}, {-12, 5}, {-5, 3}, {8, 8}, {5, 7}, {7, 2}, {9, 8},
        {-13, -4}, {-9, -5}, {-10, -12}, {-4, 2}, {-1, 12}, {-3, 1}, {2, 3}, {4, 4},
    },
    {
        {13, 24}, {15, 25}, {11, 58}, {24, 42}, {-3, 0}, {-43, -52}, {-59, -67}, {-75, -55},
        {-21, 57}, {-21, 55}, {11, 31}, {-3, 64}, {0, 22}, {-30, -15}, {-12, -51}, {-23, -38},
        {3, 23}, {12, 6}, {-7, 27}, {-12, 18}, {3, 11}, {11, -18}, {-4, -15}, {21, -34},
        {6, 12}, {-15, 14}, {-11, 2}, {-17, 14}, {-4, 11}, {13, -5}, {9, -5}, {10, -7},
        {9, -1}, {-6, 1}, {2, -6}, {-10, 6}, {10, 5}, {7, 1}, {7, 1}, {3, 5},
        {-11, -1}, {-7, -6}, {-5, -9}, {-7, 0}, {9, -2}, {-10, 1}, {8, -1}, {9, -6},
    },
    {
        {-34, -35}, {-23, 7}, {13, 38}, {11, 56}, {-4, 36}, {-12, 0}, {-51, -56}, {-64, -55},
        {-5, 6}, {-10, 25}, {-12, 60}, {5, 36}, {-4, 52}, {-6, 19}, {-11, -21}, {-18, -27},
        {-4, 2}, {-1, 15}, {15, 12}, {-19, 16}, {4, 8}, {0, 13}, {3, -8}, {14, -19},
        {7, -1}, {-1, 7}, {-5, 7}, {-7, -5}, {-8, 10}, {16, 2}, {12, -5}, {10, -5},
        {11, -1}, {-1, 1}, {-10, 3}, {0, 2}, {-4, 3}, {1, 3}, {5, 4}, {9, 3},
        {-7, 0}, {-7, -1}, {-12, -3}, {16, -12}, {-10, 5}, {-4, -1}, {0, 0}, {-4, -2},
    },
    {
        {-36, -39}, {-29, -58}, {-50, -59}, {-37, -75}, {-43, -77}, {-65, -75}, {-53, -77}, {-42, -43},
        {-14, 18}, {16, 7}, {-13, -22}, {-18, -59}, {-43, -72}, {-54, -64}, {-53, -65}, {-8, -40},
        {-2, 20}, {12, 12}, {17, 8}, {-4, -2}, {-6, -18}, {1, -24}, {-7, -26}, {9, -27},
        {10, 8}, {-8, 33}, {-10, 21}, {0, 36}, {17, 13}, {14, 10}, {12, -1}, {33, 2},
        {-4, 6}, {20, 7}, {2, 15}, {-4, 33}, {11, 29}, {17, 21}, {14, 22}, {-6, 29},
        {-3, -1}, {-40, -2}, {-22, -11}, {9, 18}, {7, 21}, {-5, 26}, {17, 14}, {-14, 22},
    },
    {
        {-14, -24}, {-34, -45}, {-39, -40}, {-50, -56}, {-53, -82}, {-53, -71}, {-58, -77}, {-46, -57},
        {21, 11}, {-7, 16}, {-2, 4}, {-25, -35}, {-28, -57}, {-28, -69}, {-45, -68}, {-31, -49},
        {-10, 14}, {19, 41}, {10, 17}, {13, 3}, {3, -16}, {21, -35}, {-7, -32}, {8, -31},
        {-6, 21}, {12, 43}, {-14, 41}, {22, 27}, {20, 24}, {39, 2}, {17, 2}, {14, 4},
        {19, 2}, {-14, 7}, {10, 14}, {7, 24}, {6, 34}, {18, 15}, {31, 11}, {7, 21},
        {-26, -11}, {-21, -9}, {-49, 4}, {-12, 8}, {7, 19}, {15, 20}, {-8, 28}, {-27, 28},
    },
    {
        {-44, -19}, {-45, -47}, {-34, -35}, {-36, -48}, {-46, -55}, {-45, -65}, {-51, -64}, {-43, -58},
        {8, 2}, {11, 3}, {3, 2}, {-7, -2}, {-11, -36}, {-20, -47}, {-35, -64}, {1, -51},
        {4, 14}, {8, 27}, {35, 22}, {13, 8}, {-3, 7}, {6, -14}, {5, -16}, {23, -31},
        {-20, 32}, {-10, 31}, {20, 32}, {32, 16}, {3, 29}, {30, 20}, {29, 2}, {10, 9},
        {-2, 17}, {2, 14}, {-15, 8}, {23, 12}, {5, 24}, {10, 20}, {-5, 22}, {4, 24},
        {-10, -3}, {-15, -3}, {-42, -7}, {-3, 0}, {-15, 13}, {10, 10}, {17, 14}, {1, 13},
    },
    {
        {-36, -45}, {-30, -42}, {-30, -51}, {-20, -42}, {-31, -50}, {-27, -54}, {-35, -56}, {-41, -58},
        {-13, -7}, {5, -21}, {7, -4}, {0, -4}, {-17, -9}, {-14, -24}, {-36, -36}, {-16, -33},
        {13, 0}, {-1, 17}, {-7, 21}, {22, 18}, {19, 2}, {31, -1}, {11, -5}, {7, -11},
        {7, 25}, {-6, 33}, {0, 28}, {10, 22}, {22, 19}, {5, 24}, {23, 20}, {12, 11},
        {1, 29}, {13, 11}, {10, 3}, {-17, 9}, {2, 16}, {11, 15}, {13, 19}, {13, 23},
        {-12, 14}, {-9, -7}, {-35, -7}, {-23, 5}, {-9, 8}, {6, -8}, {12, 11}, {-8, 14},
    },
    {
        {-20, -35}, {-50, -61}, {-27, -62}, {-43, -60}, {-46, -73}, {-52, -68}, {-52, -74}, {-30, -60},
        {16, -7}, {-33, -47}, {-10, -37}, {-26, -44}, {-58, -62}, {-46, -77}, {-49, -76}, {-18, -52},
        {7, 13}, {-10, -11}, {-11, -7}, {-8, -12}, {-13, -38}, {6, -40}, {4, -33}, {-1, -28},
        {9, 34}, {34, 35}, {-16, 4}, {45, 22}, {6, 18}, {11, 19}, {17, -6}, {22, 6},
        {11, 22}, {27, 20}, {5, 22}, {16, 27}, {5, 33}, {5, 28}, {19, 13}, {2, 24},
        {45, 51}, {12, 7}, {-8, 17}, {26, 30}, {15, 12}, {7, 39}, {28, 22}, {-15, 26},
    },
    {
        {-13, -39}, {-46, -60}, {-42, -58}, {-61, -80}, {-58, -82}, {-58, -91}, {-57, -79}, {-39, -88},
        {-20, -17}, {-24, -14}, {-18, -33}, {-24, -67}, {-38, -63}, {-30, -82}, {-27, -77}, {-35, -67},
        {10, 0}, {21, 15}, {-5, 11}, {1, -5}, {9, -26}, {1, -44}, {13, -33}, {-3, -28},
        {-15, 15}, {37, 36}, {22, 41}, {17, 8}, {8, 28}, {5, 11}, {24, 3}, {14, 4},
        {18, 17}, {15, 34}, {10, 22}, {-2, 27}, {25, 45}, {19, 22}, {34, 18}, {9, 22},
        {18, 27}, {24, 25}, {-5, 18}, {-3, 15}, {42, 19}, {24, 21}, {23, 26}, {-1, 24},
    },
    {
        {-31, -68}, {-42, -48}, {-51, -55}, {-42, -73}, {-52, -74}, {-56, -77}, {-52, -79}, {-33, -68},
        {0, -31}, {-11, -30}, {-18, -46}, {-20, -48}, {-38, -64}, {-40, -76}, {-49, -76}, {-18, -62},
        {9, 2}, {15, 16}, {23, 11}, {14, 0}, {7, -8}, {0, -35}, {19, -31}, {3, -32},
        {-7, 17}, {11, 36}, {6, 33}, {28, 21}, {7, 14}, {28, 13}, {25, 11}, {15, 2},
        {18, 12}, {18, 31}, {19, 29}, {14, 20}, {26, 21}, {8, 33}, {30, 20}, {32, 21},
        {11, 13}, {-2, 11}, {-2, 28}, {-2, 8}, {-2, 15}, {24, 24}, {20, 20}, {28, 11},
    },
    {
        {-36, -53}, {-29, -67}, {-35, -57}, {-52, -59}, {-45, -68}, {-51, -68}, {-34, -67}, {-29, -69},
        {-17, -34}, {-25, -49}, {-25, -39}, {-20, -45}, {-30, -44}, {-34, -59}, {-39, -52}, {-21, -37},
        {0, -2}, {1, -2}, {7, 8}, {7, 8}, {29, -19}, {16, -13}, {-13, -12}, {-3, -7},
        {20, 22}, {-6, 32}, {18, 29}, {28, 18}, {19, 13}, {16, 13}, {15, 27}, {9, 18},
        {25, 33}, {-17, 33}, {15, 27}, {16, 32}, {26, 19}, {23, 11}, {0, 36}, {19, 29},
        {10, 30}, {-4, 10}, {-11, 9}, {25, 5}, {-10, 10}, {12, 0}, {23, 27}, {-8, 41},
    },
};

template<Color C, PieceType Type, unsigned Offset = 0>
Score evaluate_king_relative(
#ifndef TUNE
    const
#endif
    Weight* psqt,
    unsigned flip)
{
    Score r{};
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
    unsigned king_idx16 = ((rel_king_sq & 48) >> 2) | (rel_king_sq & 3);
    unsigned king_idx32 = ((rel_king_sq & 56) >> 1) | (rel_king_sq & 3);

    Score r{};
    r += evaluate_king_relative<C, PAWN, 8>(king_relative_pawn_psqt[king_idx32], flip);
    r += evaluate_king_relative<C, KNIGHT>(king_relative_knight_psqt[king_idx16], flip);
    r += evaluate_king_relative<~C, PAWN, 8>(king_relative_opp_pawn_psqt[king_idx16], flip);
    return r;
}

#ifndef TUNE
constexpr int SCORE_MATE = 32767;
#endif
constexpr int SCORE_WIN = 32000;

std::pair<Score, bool> evaluate_main(int alpha = -SCORE_WIN, int beta = SCORE_WIN)
{
    Score lazy_eval = position.piece_square_values[position.next] - position.piece_square_values[~position.next];
    Score eval{};

#ifdef TUNE
    static_cast<void>(alpha);
    static_cast<void>(beta);
    constexpr bool lazy = false;
#else
    constexpr int lazy_threshold = 296;
    bool lazy = (lazy_eval.mid <= alpha - lazy_threshold || lazy_eval.mid >= beta + lazy_threshold) &&
            (lazy_eval.end <= alpha - lazy_threshold || lazy_eval.end >= beta + lazy_threshold);
#endif

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

    static
#ifndef TUNE
    constexpr
#endif
    Weight tempo{23, 11};
    Score result = lazy_eval + (position.next == WHITE ? eval : -eval) + tempo;
    return {result, lazy};
}

int evaluate_phase(const Position& pos, Weight result)
{
    constexpr int MIDGAME_WEIGHT = 40;
#ifdef TUNE
    constexpr int ENDGAME_WEIGHT = MIDGAME_WEIGHT;
#else
    constexpr int ENDGAME_WEIGHT = 36;
#endif

    int pieces = popcount(pos.all_bb()) + popcount(pos.all_bb() & ~pos.type_bb[PAWN]) - 2;
    int v = (MIDGAME_WEIGHT * pieces * result.mid + ENDGAME_WEIGHT * (48 - pieces) * result.end) / 1920;
    if (!pos.type_bb[PAWN])
        v = evaluate_pawnless(pos, v);
    else if (popcount(pos.all_bb()) <= 5 && popcount(pos.type_bb[PAWN]) == 1)
        v = (pos.type_bb[PAWN] & pos.color_bb[WHITE]) ? evaluate_single_pawn<WHITE>(pos, v) : evaluate_single_pawn<BLACK>(pos, v);
    return v;
}

#ifndef TUNE
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

    auto result = evaluate_main(alpha, beta);
    int v = evaluate_phase(position, result.first);

    if (!result.second)
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
        int inc = 45 * (depth - 1) + 686 - std::clamp(eval - beta - 50, 0, 600);
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
#endif

#ifdef TUNE
constexpr double lambda = .7;
constexpr double sig_r = 0.9948;

inline double sigmoid(double eval)
{
    return 1. / (1. + std::pow(sig_r, eval));
}

struct TuneVariable
{
    std::int16_t* p;
    double gradient;
    double oscillation;
    double zeros;
    std::int16_t value;
};

struct TuneFeature
{
    std::uint16_t index;
    std::int16_t value;
};

struct TunePosition
{
    Position pos;
    std::vector<TuneFeature> features;
    Weight base_score;
    double expected;
};

class Tuner
{
public:
    void tune();
    void parse_positions(std::vector<TunePosition>& positions, std::size_t size);
    std::pair<double, Weight> evaluate_features(const TunePosition& tp) const;
    double find_gradients(std::vector<TunePosition>& positions, std::size_t offset, std::size_t size);
    double evaluation_error(const std::vector<TunePosition>& positions, std::size_t offset, std::size_t size) const;
    static void print_error(double error);

    void add_variable(Weight& v);
    template<std::size_t N> void add_variables(Weight (&arr)[N]);
    template<std::size_t N, std::size_t M> void add_variables(Weight (&arr)[N][M]);
    void freeze_variables();

    template<std::size_t N> void print_variables(const char* name, Weight (&arr)[N]);
    template<std::size_t N, std::size_t M> void print_variables(const char* name, Weight (&arr)[N][M]);

    std::vector<TuneVariable> variables;
    std::unordered_map<std::int16_t*, std::uint16_t> value_indexes;
};

void Tuner::freeze_variables()
{
    for (std::size_t i = 0; i < variables.size(); i++)
        value_indexes.insert({variables[i].p, i});
}

void Tuner::parse_positions(std::vector<TunePosition>& positions, std::size_t size)
{
    auto start = Clock::now();
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
        if (position.fullmove_counter <= 6)
            continue;
        auto features = evaluate_main().first.features;
        std::sort(features.begin(), features.end(), [](const auto& lhs, const auto& rhs){ return lhs.first < rhs.first; });

        Weight base_score{};
        std::vector<TuneFeature> tune_features;
        for (auto f : features)
        {
            auto index = value_indexes.find(&f.first->mid);
            if (index == value_indexes.end())
            {
                base_score.mid += f.first->mid * f.second;
                base_score.end += f.first->end * f.second;
            }
            else
            {
                if (!tune_features.empty() && index->second == tune_features.back().index)
                    tune_features.back().value += static_cast<std::int16_t>(f.second);
                else
                    tune_features.push_back({index->second, static_cast<std::int16_t>(f.second)});
            }
        }
        tune_features.shrink_to_fit();

        for (Color c : {WHITE, BLACK})
            position.piece_square_values[c].features.clear();
        double expected = (1. - lambda) * result * 0.5 + lambda * sigmoid(score);
        positions.push_back({position, std::move(tune_features), base_score, expected});

        if (positions.size() % 500000 == 0)
        {
            double elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(Clock::now() - start).count();
            std::cout << positions.size() << " elapsed: " << elapsed << " positions per sec: " << (positions.size() / elapsed) << std::endl;
        }

        if (positions.size() >= size)
            break;
    }

    std::cout << "Read " << pos_count << " positions, using " << positions.size() << ", discarded " << (pos_count - positions.size()) << std::endl;
}

void Tuner::print_error(double error)
{
    std::cout << std::fixed << std::setprecision(4) << (100000. * error) << std::endl;
}

void add(Weight& w, int i, std::int16_t v)
{
    (i == 0 ? w.mid : w.end) += v;
}

double error_squared(const TunePosition& tp, Weight r)
{
    int v = evaluate_phase(tp.pos, r);
    double err = tp.expected - sigmoid(v);
    return err * err;
}

std::pair<double, Weight> Tuner::evaluate_features(const TunePosition& tp) const
{
    Weight r = tp.base_score;
    for (auto f : tp.features)
    {
        for (int j = 0; j < 2; j++)
            add(r, j, variables[f.index + j].value * f.value);
    }
    double err = error_squared(tp, r);
    return {err, r};
}

double Tuner::find_gradients(std::vector<TunePosition>& positions, std::size_t offset, std::size_t size)
{
    double error_sum = 0.0;
    std::vector<double> error_delta_sums(variables.size(), 0.0);

    for (std::size_t i = 0; i < size; i++)
    {
        const TunePosition& tp = positions[offset + i];
        auto r = evaluate_features(tp);
        error_sum += r.first;

        for (auto f : tp.features)
        {
            for (int j = 0; j < 2; j++)
            {
                Weight fr = r.second;
                int delta = variables[f.index + j].value >= 0 ? 1 : -1;
                add(fr, j, delta * f.value);

                error_delta_sums[f.index + j] += error_squared(tp, fr) - r.first;
            }
        }
    }

    double current_error = error_sum / size;

    for (std::size_t i = 0; i < variables.size(); i++)
    {
        TuneVariable& variable = variables[i];
        int delta = variables[i].value >= 0 ? 1 : -1;
        double gradient = -delta * error_delta_sums[i] / size;
        bool oscillation = gradient * variable.gradient < 0;
        variable.gradient = gradient;
        variable.oscillation = 0.1 * oscillation + 0.9 * variable.oscillation;
    }

    return current_error;
}

double Tuner::evaluation_error(const std::vector<TunePosition>& positions, std::size_t offset, std::size_t size) const
{
    double error_sum = std::transform_reduce(
        std::execution::par_unseq,
        positions.begin() + offset, positions.begin() + offset + size,
        0.,
        std::plus{},
        [this](const TunePosition& tp)
        {
            auto r = evaluate_features(tp);
            return r.first;
        }
    );

    return error_sum / size;
}

void Tuner::add_variable(Weight& v)
{
    for (auto* p : {&v.mid, &v.end})
    {
        *p = 0;
        variables.push_back({p, 0., 0., 0., *p});
    }
}

template<std::size_t N> void Tuner::add_variables(Weight (&arr)[N])
{
    for (Weight& v : arr)
        add_variable(v);
}

template<std::size_t N, std::size_t M> void Tuner::add_variables(Weight (&arr)[N][M])
{
    for (auto& row : arr)
        add_variables(row);
}

template<std::size_t N> void Tuner::print_variables(const char* name, Weight (&arr)[N])
{
    std::cout << "Weight " << name << "[" << N << "] =";
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

template<std::size_t N, std::size_t M> void Tuner::print_variables(const char* name, Weight (&arr)[N][M])
{
    std::cout << "Weight " << name << "[" << N << "][" << M << "] =\n";
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

    /*
    for (int i = 0; i < 5; i++)
        add_variable(material[i]);
    */
    add_variables(piece_square_table);
    add_variables(pawn_evals);
    add_variables(mobility_evals);
    add_variables(king_evals);
    add_variables(piece_evals);
    add_variables(king_relative_pawn_psqt);
    add_variables(king_relative_knight_psqt);
    add_variables(king_relative_opp_pawn_psqt);
    freeze_variables();

    std::vector<TunePosition> test_positions;
    parse_positions(test_positions, 15000000);

    std::vector<TunePosition> tuning_positions;

    std::mt19937 rng{};
    std::size_t offset = 0;
    constexpr int rounds = 800;
    constexpr double learning_rate = 500000.;

    double best_error = std::numeric_limits<double>::max();

    for (int k = 0; k < rounds; k++)
    {
        if (k % 600 == 0)
        {
            tuning_positions.clear();
            parse_positions(tuning_positions, 30000000);
        }
        std::size_t size = std::min<std::size_t>(500000U, tuning_positions.size());

        std::cout << "===== " << k << " =====" << std::endl;
        double error = find_gradients(tuning_positions, offset, size);
        print_error(error);

        double limit = 50. * (rounds - k) / rounds + 1;

        for (auto& p : variables)
        {
            double scale = (1.0 - p.oscillation) / (1.0 - p.zeros);
            std::int16_t delta = static_cast<std::int16_t>(std::clamp(learning_rate * p.gradient * scale * scale * std::exp(static_cast<double>(rounds - k) / rounds), -limit, limit));
            p.value += delta;

            p.zeros = 0.1 * (delta == 0) + 0.9 * p.zeros;
        }

        if (k % 25 == 0 || k >= rounds - 100)
        {
            double error = evaluation_error(test_positions, 0, test_positions.size());
            print_error(error);
            if (error < best_error)
            {
                std::cout << "Improved" << std::endl;
                best_error = error;
                for (auto& v : variables)
                    *v.p = v.value;
            }
        }

        offset += size;
        if (size + offset > tuning_positions.size())
        {
            offset = 0;
            std::shuffle(tuning_positions.begin(), tuning_positions.end(), rng);
        }
    }

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
