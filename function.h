#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>/* 亂數相關函數 */
#include <string.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <cstdint> 
#include <string.h>
#include <time.h> /* 時間相關函數 */

using namespace std;
// using SA = struct sockaddr;
#define MAXLINE 10000
#define SA struct sockaddr

string animals[] = { "aardvark", "aardwolf", "abalone", "acouchi", "addax", "adder", "adouri", "agama", "agouti", "aidi", "airedale", "akitainu", "albino", "alleycat", "alpaca", "ammonite", "amoeba", "amphiuma", "anaconda", "anchovy", "anemone", "angora", "anhinga", "ankole", "annelid", "annelida", "anole", "ant", "antbear", "anteater", "antelope", "antlion", "anura", "aoudad", "ape", "aphid", "aracari", "arachnid", "arawana", "argali", "arkshell", "armyant", "armyworm", "arrowana", "aruanas", "asp", "ass", "astarte", "auk", "auklet", "aurochs", "avians", "avocet", "axisdeer", "axolotl", "ayeaye", "aztecant", "babirusa", "baboon", "bactrian", "badger", "bagworm", "baiji", "banteng", "barb", "barbet", "barnacle", "barnowl", "basenji", "basil", "basilisk", "bass", "bat", "bats", "beagle", "bear", "beaver", "bedbug", "bee", "beetle", "bellfrog", "betafish", "bettong", "bighorn", "bilby", "bird", "bison", "bittern", "blackfly", "blacklab", "blesbok", "blobfish", "blowfish", "bluebird", "bluefish", "bluegill", "bluejay", "bluet", "boa", "boar", "bobcat", "bobolink", "bobwhite", "bongo", "bonobo", "booby", "borer", "borzoi", "boto", "boubou", "boutu", "bovine", "brant", "bream", "bronco", "buck", "budgie", "bufeo", "buffalo", "bug", "bull", "bullfrog", "bunny", "bunting", "burro", "bushbaby", "bustard", "buzzard", "caiman", "calf", "camel", "canary", "canine", "capybara", "caracal", "cardinal", "caribou", "carp", "cat", "catbird", "catfish", "cats", "cattle", "caudata", "cavy", "cero", "chafer", "chamois", "cheetah", "chick", "chicken", "chipmunk", "chital", "cicada", "cirriped", "civet", "clam", "clumber", "coati", "cob", "cobra", "cock", "cockatoo", "cod", "collie", "colt", "comet", "conch", "condor", "coney", "conure", "cony", "coot", "cooter", "copepod", "coqui", "coral", "cougar", "cow", "cowbird", "cowrie", "coyote", "coypu", "crab", "crane", "cranefly", "crayfish", "creature", "cricket", "crow", "cub", "cuckoo", "cur", "curassow", "curlew", "cuscus", "cutworm", "cygnet", "dairycow", "dartfrog", "dassie", "deer", "degu", "degus", "dikdik", "dikkops", "dingo", "dinosaur", "dipper", "discus", "dodo", "dodobird", "doe", "dog", "dogfish", "dolphin", "donkey", "dorado", "dore", "dorking", "dormouse", "dotterel", "dove", "dragon", "drake", "drever", "drongo", "duck", "duckling", "dugong", "duiker", "dunlin", "dunnart", "eagle", "earwig", "echidna", "eel", "eeve", "eft", "egg", "egret", "eider", "eland", "elephant", "elk", "elkhound", "elver", "emu", "equine", "erin", "ermine", "erne", "esok", "ewe", "eyas", "eyra", "fairyfly", "falcon", "fantail", "fanworms", "fawn", "feline", "ferret", "finch", "finwhale", "fireant", "firefly", "fish", "flamingo", "flatfish", "flea", "flee", "flicker", "flies", "flounder", "fluke", "fly", "foal", "fossa", "fowl", "fox", "foxhound", "frog", "fruitbat", "fruitfly", "fugu", "fulmar", "furseal", "gadwall", "galago", "galah", "gander", "gannet", "gar", "garpike", "gaur", "gavial", "gazelle", "gecko", "geese", "gelada", "gelding", "gemsbok", "gemsbuck", "genet", "gerbil", "gerenuk", "gharial", "gibbon", "giraffe", "glowworm", "gnat", "gnu", "goa", "goat", "godwit", "goitered", "goldfish", "gonolek", "goose", "gopher", "goral", "gorilla", "goshawk", "gosling", "gourami", "grackle", "grayfox", "grayling", "graywolf", "grebe", "grison", "grosbeak", "grouper", "grouse", "grub", "grunion", "guanaco", "gull", "guppy", "hackee", "haddock", "hagfish", "hake", "halcyon", "halibut", "halicore", "hamster", "hapuka", "hapuku", "hare", "harpseal", "harrier", "hart", "hawk", "hedgehog", "heifer", "hen", "herald", "heron", "herring", "hind", "hoatzin", "hog", "hogget", "hoiho", "hoki", "honeybee", "hoopoe", "hornbill", "hornet", "horse", "horsefly", "hound", "housefly", "hoverfly", "huemul", "huia", "human", "husky", "hydra", "hyena", "hyrax", "ibex", "ibis", "ibisbill", "icefish", "ichidna", "iggypops", "iguana", "imago", "impala", "incatern", "inchworm", "indri", "inganue", "insect", "isopod", "jabiru", "jackal", "jaeger", "jaguar", "javalina", "jay", "jenny", "jerboa", "joey", "johndory", "junco", "junebug", "kagu", "kakapo", "kangaroo", "karakul", "katydid", "kawala", "kestrel", "kid", "killdeer", "kingbird", "kinglet", "kinkajou", "kiskadee", "kite", "kitfox", "kitten", "kitty", "kiwi", "koala", "kob", "koi", "koodoo", "kouprey", "krill", "kudu", "lacewing", "ladybird", "ladybug", "lamb", "lamprey", "langur", "lark", "larva", "lcont", "leafbird", "leafwing", "leech", "lemming", "lemur", "leopard", "leveret", "lice", "liger", "limpet", "limpkin", "ling", "lion", "lionfish", "lizard", "llama", "lobo", "lobster", "locust", "longhorn", "longspur", "loon", "lorikeet", "loris", "louse", "lovebird", "lowchen", "lunamoth", "lungfish", "lynx", "lynx��", "macaque", "macaw", "macropod", "maggot", "magpie", "malamute", "mallard", "mamba", "mammal", "mammoth", "manatee", "mandrill", "mangabey", "manta", "mantaray", "mantid", "mantis", "manxcat", "mara", "marabou", "mare", "marlin", "marmoset", "marmot", "marten", "martin", "mastiff", "mastodon", "mayfly", "mealworm", "meerkat", "merlin", "mice", "midge", "mink", "minnow", "mite", "moa", "mole", "mollies", "mollusk", "molly", "monarch", "mongoose", "mongrel", "monkey", "moorhen", "moose", "moray", "morayeel", "morpho", "mosasaur", "mosquito", "moth", "motmot", "mouflon", "mouse", "mudpuppy", "mule", "mullet", "muntjac", "murrelet", "muskox", "muskrat", "mussel", "mustang", "mutt", "myna", "mynah", "myotis��", "nabarlek", "nag", "naga", "nagapies", "nandine", "nandoo", "nandu", "narwhal", "narwhale", "nauplius", "nautilus", "nematode", "nene", "nerka", "newt", "nightjar", "nilgai", "noctilio", "noctule", "noddy", "numbat", "nuthatch", "nutria", "nyala", "nymph", "ocelot", "octopus", "okapi", "olingo", "olm", "opossum", "orca", "oriole", "oryx", "osprey", "ostracod", "ostrich", "otter", "ovenbird", "owl", "oxen", "oxpecker", "oyster", "paca��", "panda", "pangolin", "panther", "papillon", "parakeet", "parrot", "peacock", "peafowl", "peccary", "pelican", "penguin", "perch", "pewee", "pheasant", "phoebe", "phoenix", "pig", "pigeon", "piglet", "pika", "pike", "pilchard", "pinniped", "pintail", "pipit", "piranha", "pitbull", "plankton", "platypus", "plover", "polecat", "polliwog", "polyp", "pony", "pooch", "poodle", "porpoise", "possum", "prawn", "primate", "puffer", "puffin", "pug", "pullet", "puma", "pupa", "pupfish", "puppy", "pussycat", "pygmy", "python", "quagga", "quahog", "quail", "queenant", "queenbee", "quelea", "quetzal", "quokka", "quoll", "rabbit", "raccoon", "racer", "ragfish", "rail", "ram", "raptors", "rasbora", "rat", "ratfish", "rattail", "raven", "ray", "redhead", "redpoll", "redstart", "reindeer", "reptile", "reynard", "rhea", "rhino", "ringworm", "roach", "roan", "robin", "rockrat", "rodent", "roebuck", "roller", "rook", "rooster", "sable", "saiga", "salmon", "sambar", "sardine", "sawfish", "scallop", "scarab", "scaup", "scorpion", "scoter", "screamer", "seabird", "seagull", "seahog", "seahorse", "seal", "sealion", "seaslug", "seriema", "serpent", "serval", "shark", "sheep", "shelduck", "shibainu", "shihtzu", "shoveler", "shrew", "shrike", "shrimp", "siamang", "sifaka", "silkworm", "siskin", "skimmer", "skink", "skipper", "skua", "skunk", "skylark", "sloth", "slug", "smelts", "smew", "snail", "snake", "snipe", "snowdog", "snowyowl", "songbird", "sora", "sow", "sparrow", "sphinx", "spider", "sponge", "squab", "squamata", "squeaker", "squid", "squirrel", "stag", "stallion", "starfish", "starling", "steed", "steer", "stilt", "stingray", "stinkbug", "stinkpot", "stoat", "stonefly", "stork", "stud", "sturgeon", "sunbear", "sunfish", "swallow", "swan", "swift", "tadpole", "tahr", "takin", "tamarin", "tanager", "tapaculo", "tapeworm", "tapir", "tarpan", "tarsier", "taruca", "tattler", "tayra", "teal", "tegus", "teledu", "tench", "tenrec", "termite", "tern", "terrapin", "terrier", "thrasher", "thrip", "thrush", "tick", "tiger", "tilefish", "tinamou", "titi", "titmouse", "toad", "toadfish", "tomtit��", "topi", "tortoise", "toucan", "towhee", "tragopan", "trex", "trogon", "trout", "tuatara", "tuna", "turaco", "turkey", "turtle", "uakari", "umbrette", "unau", "ungulate", "unicorn", "upupa", "urchin", "urial", "urson", "urubu", "urus", "urutu", "urva", "vaquita", "veery", "verdin", "vervet", "vicuna", "viper", "vireo", "vixen", "vole", "volvox", "vulture", "wallaby", "wallaroo", "walleye", "walrus", "warbler", "warthog", "wasp", "waterbug", "watussi", "waxwing", "weasel", "weevil", "whale", "whapuku", "whelp", "whimbrel", "whippet", "whiteeye", "whooper", "widgeon", "wildcat", "willet", "wireworm", "wisent", "wolf", "wombat", "woodcock", "worm", "wrasse", "wren", "wryneck", "wyvern", "xantus", "xeme", "xenops", "xenopus", "xenurine", "xerus", "xiphias", "xoni", "xrayfish", "yaffle", "yak", "yapok", "yardant", "yearling", "yeti", "ynambu", "yucker", "zander", "zebra", "zebu", "zenaida", "zeren", "zopilote", "zorilla" };
string split_line = "--------------------------------------------------\n";

// struct CMP {
//     const bool operator ()(int& lhs, int& rhs) const {
//         return lhs < rhs;
//     }
// };

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

int Socket(int family, int type, int protocol){
    int n;
    if( (n = socket(family, type, protocol)) < 0 ) {
        cerr << "Socket create error" << endl;
        exit(0);
    }
    cerr << "Socket create success" << endl;
    return n;
}

void Bind(int socketfd, const SA * servaddr, socklen_t servlen){
    if(bind(socketfd, servaddr, servlen) != 0){
		cerr << "Socket bind fail" << endl;
		exit(0);
	} else {
		cerr << "Socket bind success" << endl;
	}
}

void Listen(int socketfd, int backlog){
	if(listen(socketfd, backlog) != 0){
		cerr << "listen fail" << endl;
		exit(0);
	} else {
		cerr << "listen success" << endl;
	}
}

int Accept(int socketfd, SA *cliaddr, socklen_t *clilen){
	int connfd;
    if((connfd = accept(socketfd, (SA *) cliaddr, clilen)) < 0) {
        cerr << "server accept fail" << endl;
        exit(0);
    }
    return connfd;
}

void Write(int connfd, const char* write_msg, size_t write_msg_len){
    if(write(connfd, write_msg, write_msg_len) == -1){
    	// cout << "write fail" << endl;
        char error_buf[256];
        memset(error_buf, 0, sizeof(error_buf));
        perror(error_buf); // print the error msg directly to the stderr
        // cerr << strerror(errno) << endl;
        cerr << "write fail";
    	// exit(0);
    }
}

string concatenate_name(string name, int expect_len){
    string space = "";
    for(int i = 0; i < (expect_len - name.length()); i++) space += " ";
    return name + space;
}