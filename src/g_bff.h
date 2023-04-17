#if 1

#ifndef _G_BFF_
#define _G_BFF_

#pragma once

constexpr uint16_t MAP_MAX_Y = 480;
constexpr uint16_t MAP_MAX_X = 480;
constexpr uint8_t NUMSECTORS = 4;
constexpr uint8_t SECTOR_MAX_Y = 120;
constexpr uint8_t SECTOR_MAX_X = 120;

#ifdef _WIN32
#define open(name, flags, mode) _open(name, flags, mode)
#define write(handle, buffer, size) _write(handle, buffer, size)
#define read(handle, buffer, size) _read(handle, buffer, size)
#define close(handle) _close(handle)
#endif

inline size_t filesize(const char* filepath)
{
#ifdef __unix__
	struct stat fdata;
	if (stat(filepath, &fdata) == -1)
#elif defined(_WIN32)
	struct _stati64 fdata;
	if (_stati64(filepath, &fdata) == -1)
#endif
    {
		N_Error("filesize: failed to stat() file {}", filepath);
	}
	return fdata.st_size;
}

typedef struct bff_level_s bff_level_t;

#define HEADER_MAGIC 0x5f3759df
#define BFF_STR_SIZE (int)80

typedef struct bff_texture_s
{
    char *buffer;
    uint64_t fsize;
} bff_texture_t;

typedef struct bff_spawn_s
{
    char entityid[BFF_STR_SIZE+1];
	sprite_t replacement;
	sprite_t marker;
    coord_t where;
	uint8_t what;
	
	void *heap;
} bff_spawn_t;


enum : uint8_t
{
    FT_OGG,
    FT_WAV,
    FT_FLAC,
    FT_OPUS
};

typedef struct bff_script_s
{
    uint64_t fsize;
    char *filebuf;

    std::vector<std::string> funclist;
} bff_script_t;

typedef struct bff_audio_s
{
    // used in file i/o
	int32_t lvl_index; // equal to -1 if not a level-specific track
    uint8_t type;
	uint64_t fsize;
    char *filebuf;

    eastl::vector<int16_t, nomad_allocator<int16_t>> buffer;
} bff_audio_t;

typedef struct bff_level_s
{
	sprite_t lvl_map[NUMSECTORS][SECTOR_MAX_Y][SECTOR_MAX_X];
	uint16_t *spawnlist;
    uint16_t spawncount;

	bff_audio_t *tracks;
	bff_spawn_t *spawns;
} bff_level_t;
typedef struct bffinfo_s
{
	uint64_t magic = HEADER_MAGIC;
	uint16_t numlevels;
	uint16_t numspawns;
    uint16_t numtextures;
	uint16_t numsounds;
} bffinfo_t;

typedef struct bff_file_s
{
	FILE* fp;
	bffinfo_t header;
	bff_spawn_t* spawns;
	bff_audio_t* sounds;
	bff_level_t* levels;
    bff_texture_t* textures;
} bff_file_t;

extern uint64_t extra_heap;
extern bff_file_t* bff;
extern bff_level_t* levels;
extern bff_texture_t* textures;
extern bff_spawn_t* spawns;
extern bffinfo_t bffinfo;

void G_LoadBFF(const std::string& bffname);
void G_ExtractBFF(const std::string& filepath);
void G_WriteBFF(const char* outfile, const char* dirname);

/**
 * @brief Huffman compression namespace
 */
namespace Huffman {

inline std::string HuffmanValue[256] = {""};

/// @brief structure for storing nodes.
struct Node {
    char character;
    uint64_t count;
    Node *left, *right;

    Node(uint64_t count) {
        this->character = 0;
        this->count = count;
        this->left = this->right = nullptr;
    }

    Node(char character, uint64_t count) {
        this->character = character;
        this->count = count;
        this->left = this->right = nullptr;
    }
};
/**
 * @brief Common function necessary for both compression and decompression.
 */
namespace Utility {
    /**
     * @brief Get size of the file
     * @param filename name of the file.
     * @returns the filesize
     */
    inline uint64_t get_file_size(const char *filename) {
        FILE *p_file = fopen(filename, "rb");
        fseek(p_file, 0, SEEK_END);
        uint64_t size = ftello64(p_file);
        fclose(p_file);
        return size;
    }
    /**
     * @brief Test function to print32_t huffman codes for each character. 
     */
    inline void Inorder(Node *root, std::string &value) {
        if (root) {
            value.push_back('0');
            Inorder(root->left, value);
            value.pop_back();

            if (root->left == nullptr && root->right == nullptr) {
                printf("Character: %c, Count: %lu, ", root->character, root->count);
                std::cout << "Huffman Value: " << value << std::endl;
            }
            
            value.push_back('1');
            Inorder(root->right, value);
            value.pop_back();
        }
    }
};

/**
 * @brief Functions necessary for compression.
 */
namespace CompressUtility {

/**
 * @brief Combine two nodes
 * @param a first node
 * @param b second node
 * @returns a node with a left and b right child.
 */
inline Node *combine(Node *a, Node *b) {
    Node *parent = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &parent);
    new (parent) Node((a ? a->count : 0)+(b ? b->count : 0));
    parent->left = b;
    parent->right = a;
    return parent;
}

/**
 * @brief comparison function.
 * @param a first node
 * @param b second node
 * @returns true if first node is greater
 */
inline bool sortbysec(const Node *a, const Node *b) { 
    return (a->count > b->count); 
}

/**
 * @details Parses the file for character count
 * @param filename name of the file.
 * @param Filesize size of the file.
 * @returns count of auint64_t present characters in file as a map
*/
inline std::map<char, uint64_t> parse_file(FILE* fp, const uint64_t Filesize)
{
    uint8_t ch;
    uint64_t size = 0, filesize = Filesize;
    std::vector<uint64_t> Store(256, 0);

    while (size != filesize) {
        ch = fgetc(fp);
        ++Store[ch];
        ++size;
    }

    std::map<char, uint64_t> store;
    for (int32_t i = 0; i < 256; ++i) {
        if (Store[i]) {
            store[i] = Store[i];
        }
    }
    return store;
}
inline std::map<char, uint64_t> parse_file(const char* filename, const uint64_t Filesize)
{
    FILE *ptr = fopen(filename, "rb");
    if (!ptr)
		N_Error("HFM_ParseFile: failed to open readonly filestream for %s", filename);

    uint8_t ch;
    uint64_t size = 0, filesize = Filesize;
    std::vector<uint64_t> Store(256, 0);

    while (size != filesize) {
        ch = fgetc(ptr);
        ++Store[ch];
        ++size;
    }

    std::map<char, uint64_t> store;
    for (int32_t i = 0; i < 256; ++i) {
        if (Store[i]) {
            store[i] = Store[i];
        }
    }
    fclose(ptr);
    return store;
}
/**
 * @details Utility function to sort array by character count
 */
static std::vector<Node*> sort_by_character_count(const std::map<char, uint64_t>&value)
{
    std::vector<Node*> store;
    for (auto &x: value) {
		Node* node = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &node);
		new (node) Node(x.first, x.second);
        store.push_back(node);
    }
    sort(store.begin(), store.end(), sortbysec);
    return store;
}
/**
 * @brief Generate a header for the file.
 * Format: 
 * 1. Total Unique Character (1 byte)
 * 2. For each unique character:
 * 2a. Character (1 byte)
 * 2b. Length of code (1 byte)
 * 2c. Huffman code (min: 1 byte, max: 255bytes)
 * 3. Padding
 * Worst case header size: 1 + (1+1)*(1+2+3+4+5+...+255) + 1 ~ 32kb... (only happens when skewed Huffman tree is generated)
 * Best case header size: 1 + 1 + 1 + 1 + 1 = 5bytes (Happens only when a single character exists in an entire file).
 */
inline std::string generate_header(const char padding)
{
    std::string header = "";
    // UniqueCharacter start from -1 {0 means 1, 1 means 2, to conserve memory}
    unsigned char UniqueCharacter = 255;
    
    for (int32_t i = 0; i < 256; ++i) {
        if (HuffmanValue[i].size()) {
            header.push_back(i);
            header.push_back(HuffmanValue[i].size());
            header += HuffmanValue[i];
            ++UniqueCharacter;
        }
    }
    char value = UniqueCharacter;
    return value+header+(char)padding;
}

/**
 * @details Store Huffman values for each character in string. 
 * @param root root of the huffman tree
 * @param value binary string
 * @returns the size of the resulting file (without the header)
 */
inline uint64_t store_huffman_value(const Node *root, std::string &value)
{
    uint64_t temp = 0;  
    if (root) {
        value.push_back('0');
        temp = store_huffman_value(root->left, value);
        value.pop_back();
        if (!root->left && !root->right) {
            HuffmanValue[(unsigned char)root->character] = value;
            temp += value.size() * root->count;
        }
        value.push_back('1');
        temp += store_huffman_value(root->right, value);
        value.pop_back();
    }
    return temp;
}

/**
 * @details Create huffman tree during compression...
 * @param value mapping of character counts.
 * @returns root of the huffman tree.
 */
inline Node *generate_huffman_tree(const std::map <char, uint64_t>& value)
{
    std::vector<Node*> store = sort_by_character_count(value);
    Node *one, *two, *parent;
    sort(begin(store), end(store), sortbysec);
    if (store.size() == 1) {
        return combine(store.back(), nullptr);
    }
    while (store.size() > 2) {
        one = *(store.end() - 1); two = *(store.end() - 2);
        parent = combine(one, two);
        store.pop_back(); store.pop_back();
        store.push_back(parent);

        std::vector<Node*>::iterator it1 = store.end() - 2;
        while ((*it1)->count < parent->count && it1 != begin(store)) {
            --it1;
        }
        std::sort(it1, store.end(), sortbysec);
    }
    one = *(store.end() - 1); two = *(store.end() - 2);
    return combine(one, two);
}
/**
 * @brief Actual compression of a file.
 * @param filename file to be compressed.
 * @param Filesize size of the file.
 * @param PredictedFileSize the size of the compressed file.
 * @returns void, but compresses the file as ${filename}.abiz
 */
inline std::vector<char> compress(FILE* memfp, const uint64_t Filesize, const uint64_t PredictedFileSize)
{
    const char padding = (8 - ((PredictedFileSize) & (7))) & (7);
    const std::string header = generate_header(padding);
    int32_t header_i = 0;
    const int32_t h_length = header.size();
    LOG_INFO("padding size: {}", (int32_t)padding);
    std::vector<char> buffer;

    while (header_i < h_length) {
        buffer.emplace_back(header[header_i]);
        ++header_i;
    }

    uint8_t ch, fch = 0;
    char counter = 7;
    uint64_t size = 0, i;
    while (size != Filesize) {
        ch = fgetc(memfp);
        i = 0;
        const std::string& huffmanstr = HuffmanValue[ch];
        while (huffmanstr[i] != '\0') {
            fch |= ((huffmanstr[i] - 48) << counter);
            counter = (counter + 7) & 7;
            if (counter == 7) {
                buffer.emplace_back(fch);
                fch ^= fch;
            }
            ++i;
        }
        ++size;
#ifdef _NOMAD_DEBUG
        if((size * 100 / Filesize) > ((size - 1) * 100 / Filesize)) {
            printf("\r%lu%% completed  ", (size * 100 / Filesize));
        }
#endif
    }
    if (fch) {
        buffer.emplace_back(fch);
    }
#ifdef _NOMAD_DEBUG
    printf("\n");
#endif
    return buffer;
}
inline void compress(const char *filename, const uint64_t Filesize, const uint64_t PredictedFileSize, std::vector<char>& outbuffer)
{
    const char padding = (8 - ((PredictedFileSize)&(7)))&(7);
    const std::string header = generate_header(padding);
    int32_t header_i = 0;
    const int32_t h_length = header.size();
    LOG_INFO("padding size: {}", (int32_t)padding);
    FILE *iptr = fopen(filename, "rb");
    
    if (!iptr)
		N_Error("HFM_Compress: failed to open readonly filestream for %s", filename);

    while (header_i < h_length) {
		outbuffer.emplace_back(header[header_i]);
        ++header_i;
    }

    uint8_t ch, fch = 0;
    char counter = 7;
    uint64_t size = 0, i;
    while(size != Filesize) {
        ch = fgetc(iptr);
        i = 0;
        const std::string &huffmanStr = HuffmanValue[ch];
        while(huffmanStr[i] != '\0') {
            fch |= ((huffmanStr[i] - '0') << counter);
            // Decrement from 7 down to zero, and then
            // back again at 7
            counter = (counter + 7) & 7;
            if(counter == 7) {
				outbuffer.emplace_back(fch);
                fch ^= fch;
            }
            ++i;
        }
        ++size;
#ifdef _NOMAD_DEBUG
        if((size * 100 / Filesize) > ((size - 1) * 100 / Filesize)) {
            printf("\r%lu%% completed  ", (size * 100 / Filesize));
        }
#endif
	}
    if(fch) {
		outbuffer.emplace_back(fch);
    }
#ifdef _NOMAD_DEBUG
    printf("\n");
#endif
    fclose(iptr);
}

};
/**
 * @brief Functions necessary for decompression.
 */
namespace DecompressUtility {
/**
 * @details Generate huffman tree based on header content
 */
inline void generate_huffman_tree(Node * const root, const std::string &codes, const unsigned char ch) {
    Node *traverse = root;
    int32_t i = 0;
    while(codes[i] != '\0') {
        if(codes[i] == '0') {
            if(!traverse->left) {
                traverse->left = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &traverse->left);
				new (traverse->left) Node(0);
            }
            traverse = traverse->left;
        } else {
            if(!traverse->right) {
                traverse->right = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &traverse->right);
				new (traverse->right) Node(0);
            }
            traverse = traverse->right;
        }
        ++i;
    }
    traverse->character = ch;
}
/**
 * @brief Function to store and generate a tree
 * @param iptr file pointer
 * @returns root of the node and pair of values,
 * first containing padding to complete a byte and 
 * total_size
 */
inline std::pair<Node*, std::pair<uint8_t, int32_t>> decode_header(FILE *iptr)
{
    Node *root = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &root);
	new (root) Node(0);
    int32_t charactercount, total_length = 1;
    size_t buffer;
    char ch, len;
    charactercount = fgetc(iptr);
    std::string codes;
    ++charactercount;
    while (charactercount) {
        ch = fgetc(iptr);
        codes = ""; 
        len = fgetc(iptr);
        buffer = len;

        while (buffer > codes.size()) {
            codes.push_back(fgetc(iptr));
        }
        // character (1byte) + length(1byte) + huffmancode(n bytes where n is length of huffmancode)
        total_length += codes.size()+2;

        generate_huffman_tree(root, codes, ch);
        --charactercount;
    }
    uint8_t padding = fgetc(iptr);
    ++total_length;
    return {root, {padding, total_length}};
}
/**
 * @details Decompresses the given .abiz file.
 * @param filename name of the file
 * @param Filesize file size
 * @param leftover 
 * @returns void, but decompresses the file and stores it as
 * output${filename} (without the .abiz part)
 */
inline std::vector<char> decompress(FILE* memfp, const uint64_t Filesize, const uint64_t leftover)
{
    std::pair<Node*, std::pair<uint8_t, int32_t>> header_meta = decode_header(memfp);
    Node *const root = header_meta.first;
    const auto [padding, headersize] = header_meta.second;
    std::vector<char> buffer;

    char ch, counter = 7;
    uint64_t size = 0;
    const uint64_t filesize = Filesize - headersize;
    Node *traverse = root;
    ch = fgetc(memfp);
    while (size != filesize) {
        while (counter >= 0) {
            traverse = ch & (1 << counter) ? traverse->right : traverse->left;
            ch ^= (1 << counter);
            --counter;
            if (!traverse->left && !traverse->right) {
                buffer.emplace_back(traverse->character);
                if(size == filesize - 1 && padding == counter + 1) {
                    break;
                }
                traverse = root;
            }
        }
        ++size;
        counter = 7;
#ifdef _NOMAD_DEBUG
        if ((size * 100 / filesize) > ((size - 1) * 100 / filesize)) {
            printf("\r%lu%% completed, size: %lu bytes   ", (size * 100 / filesize), size);
        }
#endif
        ch = fgetc(memfp);
    }
    return buffer;
}
inline void decompress(const char* filename, const uint64_t Filesize, const uint64_t leftover, std::vector<char>& outbuffer)
{
    const std::string fl = filename;
    FILE *iptr = fopen(filename, "rb");
    
    if (!iptr)
		N_Error("HFM_Decompress: failed to open input filestream buffer");

    std::pair<Node*, std::pair<uint8_t, int32_t>> HeaderMetadata = decode_header(iptr);
    Node *const root = HeaderMetadata.first;
    const auto [padding, headersize] = HeaderMetadata.second;

    char ch, counter = 7;
    uint64_t size = 0;
    const uint64_t filesize = Filesize-headersize;
    Node *traverse = root;
    ch = fgetc(iptr);
    while (size != filesize) {
        while (counter >= 0) {
            traverse = ch & (1 << counter) ? traverse->right : traverse->left;
            ch ^= (1 << counter);
            --counter;
            if (!traverse->left && !traverse->right) {
                outbuffer.emplace_back(traverse->character);
                if(size == filesize - 1 && padding == counter + 1) {
                    break;
                }
                traverse = root;
            }
        }
        ++size;
        counter = 7;
#ifdef _NOMAD_DEBUG
        if ((size * 100 / filesize) > ((size - 1) * 100 / filesize)) {
            printf("\r%lu%% completed, size: %lu bytes   ", (size * 100 / filesize), size);
        }
#endif
        ch = fgetc(iptr);
    }
    fclose(iptr);
}

} /// namespace DecompressUtility

}; /// namespace Huffman

#endif

#else

#ifndef _G_BFF_
#define _G_BFF_

#pragma once

constexpr uint16_t MAP_MAX_Y = 480;
constexpr uint16_t MAP_MAX_X = 480;
constexpr uint8_t NUMSECTORS = 4;
constexpr uint8_t SECTOR_MAX_Y = 120;
constexpr uint8_t SECTOR_MAX_X = 120;

inline size_t filesize(const char* filepath)
{
#ifdef __unix__
	struct stat fdata;
	if (stat(filepath, &fdata) == -1)
#elif defined(_WIN32)
	struct _stati64 fdata;
	if (_stati64(filepath, &fdata) == -1)
#endif
    {
		N_Error("filesize: failed to stat() file {}", filepath);
	}
	return fdata.st_size;
}

typedef struct bff_level_s bff_level_t;

#define HEADER_MAGIC 0x5f3759df
#define BFF_STR_SIZE (int)80

typedef struct bff_texture_s
{
    char name[BFF_STR_SIZE];
    char *buffer;
    uint64_t fsize;
} bff_texture_t;

typedef struct bff_spawn_s
{
	char name[BFF_STR_SIZE];
    char entityid[BFF_STR_SIZE];
	sprite_t replacement;
	sprite_t marker;
    coord_t where;
	uint8_t what;
	
	void *heap;
} bff_spawn_t;


enum : uint8_t
{
    FT_OGG,
    FT_WAV,
};

typedef struct bff_script_s
{
    char name[BFF_STR_SIZE];
    uint64_t fsize;
    char *filebuf;
	
	std::unordered_map<std::string, void *> funclist;
} bff_script_t;

typedef struct bff_audio_s
{
    // used in file i/o
    char name[BFF_STR_SIZE];
	int32_t lvl_index; // equal to -1 if not a level-specific track
	uint64_t fsize;
    int channels;
    int samplerate;

    // not used in file i/o
    eastl::vector<int16_t, zone_allocator<int16_t>> buffer;
	bff_level_t* level;
} bff_audio_t;

enum : uint8_t
{
	REWARD_GOLD,
	REWARD_XP,
	
	NUMREWARDS
};

enum : uint8_t
{
	TIME_F,
	TIME_D,
	TIME_C,
	TIME_B,
	TIME_A,
	TIME_S,
	NUMTIMES
};

typedef struct bff_level_s
{
    char name[BFF_STR_SIZE];
	sprite_t lvl_map[NUMSECTORS][SECTOR_MAX_Y][SECTOR_MAX_X];
	uint64_t times[NUMTIMES][4];
	uint32_t rewards[NUMREWARDS];
	uint64_t numspawns;
	uint16_t *spawnlist;
	
	bff_audio_t *tracks;
	bff_spawn_t *spawns;
} bff_level_t;
typedef struct bffinfo_s
{
	uint64_t magic = HEADER_MAGIC;
	char name[256];
	uint8_t compression;
	uint16_t numlevels;
	uint16_t numspawns;
    uint16_t numtextures;
	uint16_t numsounds;
} bffinfo_t;

typedef struct bff_file_s
{
	FILE* fp;
	bffinfo_t header;
	bff_spawn_t* spawns;
	bff_audio_t* sounds;
	bff_level_t* levels;
    bff_texture_t* textures;
} bff_file_t;

extern bff_file_t* bff;

void G_LoadBFF();
void G_WriteBFF(const char* outfile, const char* dir, int compression);

/**
 * @brief Huffman compression namespace
 */
namespace Huffman {

inline std::string HuffmanValue[256] = {""};

/// @brief structure for storing nodes.
struct Node {
    char character;
    uint64_t count;
    Node *left, *right;

    Node(uint64_t count) {
        this->character = 0;
        this->count = count;
        this->left = this->right = nullptr;
    }

    Node(char character, uint64_t count) {
        this->character = character;
        this->count = count;
        this->left = this->right = nullptr;
    }
};
/**
 * @brief Common function necessary for both compression and decompression.
 */
namespace Utility {
    /**
     * @brief Get size of the file
     * @param filename name of the file.
     * @returns the filesize
     */
    inline uint64_t get_file_size(const char *filename) {
        FILE *p_file = fopen(filename, "rb");
        fseek(p_file, 0, SEEK_END);
        uint64_t size = ftello64(p_file);
        fclose(p_file);
        return size;
    }
    /**
     * @brief Test function to print32_t huffman codes for each character. 
     */
    inline void Inorder(Node *root, std::string &value) {
        if (root) {
            value.push_back('0');
            Inorder(root->left, value);
            value.pop_back();

            if (root->left == nullptr && root->right == nullptr) {
                printf("Character: %c, Count: %lu, ", root->character, root->count);
                std::cout << "Huffman Value: " << value << std::endl;
            }
            
            value.push_back('1');
            Inorder(root->right, value);
            value.pop_back();
        }
    }
};

/**
 * @brief Functions necessary for compression.
 */
namespace CompressUtility {

/**
 * @brief Combine two nodes
 * @param a first node
 * @param b second node
 * @returns a node with a left and b right child.
 */
inline Node *combine(Node *a, Node *b) {
    Node *parent = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &parent);
    new (parent) Node((a ? a->count : 0)+(b ? b->count : 0));
    parent->left = b;
    parent->right = a;
    return parent;
}

/**
 * @brief comparison function.
 * @param a first node
 * @param b second node
 * @returns true if first node is greater
 */
inline bool sortbysec(const Node *a, const Node *b) { 
    return (a->count > b->count); 
}

/**
 * @details Parses the file for character count
 * @param filename name of the file.
 * @param Filesize size of the file.
 * @returns count of auint64_t present characters in file as a map
*/
inline std::map<char, uint64_t> parse_file(FILE* fp, const uint64_t Filesize)
{
    uint8_t ch;
    uint64_t size = 0, filesize = Filesize;
    std::vector<uint64_t> Store(256, 0);

    while (size != filesize) {
        ch = fgetc(fp);
        ++Store[ch];
        ++size;
    }

    std::map<char, uint64_t> store;
    for (int32_t i = 0; i < 256; ++i) {
        if (Store[i]) {
            store[i] = Store[i];
        }
    }
    return store;
}
inline std::map<char, uint64_t> parse_file(const char* filename, const uint64_t Filesize)
{
    FILE *ptr = fopen(filename, "rb");
    if (!ptr)
		N_Error("HFM_ParseFile: failed to open readonly filestream for %s", filename);

    uint8_t ch;
    uint64_t size = 0, filesize = Filesize;
    std::vector<uint64_t> Store(256, 0);

    while (size != filesize) {
        ch = fgetc(ptr);
        ++Store[ch];
        ++size;
    }

    std::map<char, uint64_t> store;
    for (int32_t i = 0; i < 256; ++i) {
        if (Store[i]) {
            store[i] = Store[i];
        }
    }
    fclose(ptr);
    return store;
}
/**
 * @details Utility function to sort array by character count
 */
static std::vector<Node*> sort_by_character_count(const std::map<char, uint64_t>&value)
{
    std::vector<Node*> store;
    for (auto &x: value) {
		Node* node = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &node);
		new (node) Node(x.first, x.second);
        store.push_back(node);
    }
    sort(store.begin(), store.end(), sortbysec);
    return store;
}
/**
 * @brief Generate a header for the file.
 * Format: 
 * 1. Total Unique Character (1 byte)
 * 2. For each unique character:
 * 2a. Character (1 byte)
 * 2b. Length of code (1 byte)
 * 2c. Huffman code (min: 1 byte, max: 255bytes)
 * 3. Padding
 * Worst case header size: 1 + (1+1)*(1+2+3+4+5+...+255) + 1 ~ 32kb... (only happens when skewed Huffman tree is generated)
 * Best case header size: 1 + 1 + 1 + 1 + 1 = 5bytes (Happens only when a single character exists in an entire file).
 */
inline std::string generate_header(const char padding)
{
    std::string header = "";
    // UniqueCharacter start from -1 {0 means 1, 1 means 2, to conserve memory}
    unsigned char UniqueCharacter = 255;
    
    for (int32_t i = 0; i < 256; ++i) {
        if (HuffmanValue[i].size()) {
            header.push_back(i);
            header.push_back(HuffmanValue[i].size());
            header += HuffmanValue[i];
            ++UniqueCharacter;
        }
    }
    char value = UniqueCharacter;
    return value+header+(char)padding;
}

/**
 * @details Store Huffman values for each character in string. 
 * @param root root of the huffman tree
 * @param value binary string
 * @returns the size of the resulting file (without the header)
 */
inline uint64_t store_huffman_value(const Node *root, std::string &value)
{
    uint64_t temp = 0;  
    if (root) {
        value.push_back('0');
        temp = store_huffman_value(root->left, value);
        value.pop_back();
        if (!root->left && !root->right) {
            HuffmanValue[(unsigned char)root->character] = value;
            temp += value.size() * root->count;
        }
        value.push_back('1');
        temp += store_huffman_value(root->right, value);
        value.pop_back();
    }
    return temp;
}

/**
 * @details Create huffman tree during compression...
 * @param value mapping of character counts.
 * @returns root of the huffman tree.
 */
inline Node *generate_huffman_tree(const std::map <char, uint64_t>& value)
{
    std::vector<Node*> store = sort_by_character_count(value);
    Node *one, *two, *parent;
    sort(begin(store), end(store), sortbysec);
    if (store.size() == 1) {
        return combine(store.back(), nullptr);
    }
    while (store.size() > 2) {
        one = *(store.end() - 1); two = *(store.end() - 2);
        parent = combine(one, two);
        store.pop_back(); store.pop_back();
        store.push_back(parent);

        std::vector<Node*>::iterator it1 = store.end() - 2;
        while ((*it1)->count < parent->count && it1 != begin(store)) {
            --it1;
        }
        std::sort(it1, store.end(), sortbysec);
    }
    one = *(store.end() - 1); two = *(store.end() - 2);
    return combine(one, two);
}
/**
 * @brief Actual compression of a file.
 * @param filename file to be compressed.
 * @param Filesize size of the file.
 * @param PredictedFileSize the size of the compressed file.
 * @returns void, but compresses the file as ${filename}.abiz
 */
inline std::vector<char> compress(FILE* memfp, const uint64_t Filesize, const uint64_t PredictedFileSize)
{
    const char padding = (8 - ((PredictedFileSize) & (7))) & (7);
    const std::string header = generate_header(padding);
    int32_t header_i = 0;
    const int32_t h_length = header.size();
    LOG_INFO("padding size: {}", (int32_t)padding);
    std::vector<char> buffer;

    while (header_i < h_length) {
        buffer.emplace_back(header[header_i]);
        ++header_i;
    }

    uint8_t ch, fch = 0;
    char counter = 7;
    uint64_t size = 0, i;
    while (size != Filesize) {
        ch = fgetc(memfp);
        i = 0;
        const std::string& huffmanstr = HuffmanValue[ch];
        while (huffmanstr[i] != '\0') {
            fch |= ((huffmanstr[i] - 48) << counter);
            counter = (counter + 7) & 7;
            if (counter == 7) {
                buffer.emplace_back(fch);
                fch ^= fch;
            }
            ++i;
        }
        ++size;
#ifdef _NOMAD_DEBUG
        if((size * 100 / Filesize) > ((size - 1) * 100 / Filesize)) {
            printf("\r%lu%% completed  ", (size * 100 / Filesize));
        }
#endif
    }
    if (fch) {
        buffer.emplace_back(fch);
    }
#ifdef _NOMAD_DEBUG
    printf("\n");
#endif
    return buffer;
}
inline void compress(const char *filename, const uint64_t Filesize, const uint64_t PredictedFileSize, std::vector<char>& outbuffer)
{
    const char padding = (8 - ((PredictedFileSize)&(7)))&(7);
    const std::string header = generate_header(padding);
    int32_t header_i = 0;
    const int32_t h_length = header.size();
    LOG_INFO("padding size: {}", (int32_t)padding);
    FILE *iptr = fopen(filename, "rb");
    
    if (!iptr)
		N_Error("HFM_Compress: failed to open readonly filestream for %s", filename);

    while (header_i < h_length) {
		outbuffer.emplace_back(header[header_i]);
        ++header_i;
    }

    uint8_t ch, fch = 0;
    char counter = 7;
    uint64_t size = 0, i;
    while(size != Filesize) {
        ch = fgetc(iptr);
        i = 0;
        const std::string &huffmanStr = HuffmanValue[ch];
        while(huffmanStr[i] != '\0') {
            fch |= ((huffmanStr[i] - '0') << counter);
            // Decrement from 7 down to zero, and then
            // back again at 7
            counter = (counter + 7) & 7;
            if(counter == 7) {
				outbuffer.emplace_back(fch);
                fch ^= fch;
            }
            ++i;
        }
        ++size;
#ifdef _NOMAD_DEBUG
        if((size * 100 / Filesize) > ((size - 1) * 100 / Filesize)) {
            printf("\r%lu%% completed  ", (size * 100 / Filesize));
        }
#endif
	}
    if(fch) {
		outbuffer.emplace_back(fch);
    }
#ifdef _NOMAD_DEBUG
    printf("\n");
#endif
    fclose(iptr);
}

};
/**
 * @brief Functions necessary for decompression.
 */
namespace DecompressUtility {
/**
 * @details Generate huffman tree based on header content
 */
inline void generate_huffman_tree(Node * const root, const std::string &codes, const unsigned char ch) {
    Node *traverse = root;
    int32_t i = 0;
    while(codes[i] != '\0') {
        if(codes[i] == '0') {
            if(!traverse->left) {
                traverse->left = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &traverse->left);
				new (traverse->left) Node(0);
            }
            traverse = traverse->left;
        } else {
            if(!traverse->right) {
                traverse->right = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &traverse->right);
				new (traverse->right) Node(0);
            }
            traverse = traverse->right;
        }
        ++i;
    }
    traverse->character = ch;
}
/**
 * @brief Function to store and generate a tree
 * @param iptr file pointer
 * @returns root of the node and pair of values,
 * first containing padding to complete a byte and 
 * total_size
 */
inline std::pair<Node*, std::pair<uint8_t, int32_t>> decode_header(FILE *iptr)
{
    Node *root = (Node *)Z_Malloc(sizeof(Node), TAG_NODE, &root);
	new (root) Node(0);
    int32_t charactercount, total_length = 1;
    size_t buffer;
    char ch, len;
    charactercount = fgetc(iptr);
    std::string codes;
    ++charactercount;
    while (charactercount) {
        ch = fgetc(iptr);
        codes = ""; 
        len = fgetc(iptr);
        buffer = len;

        while (buffer > codes.size()) {
            codes.push_back(fgetc(iptr));
        }
        // character (1byte) + length(1byte) + huffmancode(n bytes where n is length of huffmancode)
        total_length += codes.size()+2;

        generate_huffman_tree(root, codes, ch);
        --charactercount;
    }
    uint8_t padding = fgetc(iptr);
    ++total_length;
    return {root, {padding, total_length}};
}
/**
 * @details Decompresses the given .abiz file.
 * @param filename name of the file
 * @param Filesize file size
 * @param leftover 
 * @returns void, but decompresses the file and stores it as
 * output${filename} (without the .abiz part)
 */
inline std::vector<char> decompress(FILE* memfp, const uint64_t Filesize, const uint64_t leftover)
{
    std::pair<Node*, std::pair<uint8_t, int32_t>> header_meta = decode_header(memfp);
    Node *const root = header_meta.first;
    const auto [padding, headersize] = header_meta.second;
    std::vector<char> buffer;

    char ch, counter = 7;
    uint64_t size = 0;
    const uint64_t filesize = Filesize - headersize;
    Node *traverse = root;
    ch = fgetc(memfp);
    while (size != filesize) {
        while (counter >= 0) {
            traverse = ch & (1 << counter) ? traverse->right : traverse->left;
            ch ^= (1 << counter);
            --counter;
            if (!traverse->left && !traverse->right) {
                buffer.emplace_back(traverse->character);
                if(size == filesize - 1 && padding == counter + 1) {
                    break;
                }
                traverse = root;
            }
        }
        ++size;
        counter = 7;
#ifdef _NOMAD_DEBUG
        if ((size * 100 / filesize) > ((size - 1) * 100 / filesize)) {
            printf("\r%lu%% completed, size: %lu bytes   ", (size * 100 / filesize), size);
        }
#endif
        ch = fgetc(memfp);
    }
    return buffer;
}
inline void decompress(const char* filename, const uint64_t Filesize, const uint64_t leftover, std::vector<char>& outbuffer)
{
    const std::string fl = filename;
    FILE *iptr = fopen(filename, "rb");
    
    if (!iptr)
		N_Error("HFM_Decompress: failed to open input filestream buffer");

    std::pair<Node*, std::pair<uint8_t, int32_t>> HeaderMetadata = decode_header(iptr);
    Node *const root = HeaderMetadata.first;
    const auto [padding, headersize] = HeaderMetadata.second;

    char ch, counter = 7;
    uint64_t size = 0;
    const uint64_t filesize = Filesize-headersize;
    Node *traverse = root;
    ch = fgetc(iptr);
    while (size != filesize) {
        while (counter >= 0) {
            traverse = ch & (1 << counter) ? traverse->right : traverse->left;
            ch ^= (1 << counter);
            --counter;
            if (!traverse->left && !traverse->right) {
                outbuffer.emplace_back(traverse->character);
                if(size == filesize - 1 && padding == counter + 1) {
                    break;
                }
                traverse = root;
            }
        }
        ++size;
        counter = 7;
#ifdef _NOMAD_DEBUG
        if ((size * 100 / filesize) > ((size - 1) * 100 / filesize)) {
            printf("\r%lu%% completed, size: %lu bytes   ", (size * 100 / filesize), size);
        }
#endif
        ch = fgetc(iptr);
    }
    fclose(iptr);
}

} /// namespace DecompressUtility

}; /// namespace Huffman

#endif

#endif