#include <iostream>      // cout, cerr için
#include <string>        // std::string için
#include <filesystem>    // os.path.exists yerine kullanılır (C++17)
#include <cmath>         // math modülünün karşılığı (ceil için)
#include <fstream>
// macOS BSD socket header'ları (sistemin kendi socket API'si):
#include <sys/socket.h>  // socket(), bind(), sendto() vs.
#include <netinet/in.h>  // sockaddr_in, INADDR_BROADCAST gibi yapılar
#include <arpa/inet.h>   // inet_addr(), inet_pton() gibi IP dönüşüm fonksiyonları
#include <unistd.h>         // close() — soketi kapatmak için
#include <vector>
#include <nlohmann/json.hpp>
namespace fs = std::filesystem;
using json = nlohmann::json;

const std::string BROADCAST_IP = "192.168.1.255";  // LAN broadcast address; every device on 192.168.1.x receives this
const int UDP_PORT = 6000;// destination port all Content Discovery processes listen on

sockaddr_in broadcastAddr{};


// Divides the given file into exactly 3 equal-sized chunks and saves them to disk.
std::string split_file(const std::string& file_path) {
    if (!fs::exists(file_path)) {                    // check whether the specified file actually exists on disk
        std::cerr << "\n[!] ERROR: '" << file_path << "' does not exist in your folder.\n";
        return "";
    }
    int file_size = fs::file_size(file_path);        
    int chunk_size = file_size / 3;
    std::string base_name = fs::path(file_path).filename().string();

    std::cout<<"[*] Splitting "+base_name+" into 3 physical chunks..."<<std::endl;

    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::cerr << "\n[!] ERROR: '" << file_path << "' açılamadı.\n";
        return "";
    }
    std::vector<char> buffer(chunk_size); 
    for(int i =1;i < 4;i++){

        std::string chunk_filename = base_name +"_" + std::to_string(i);
        file.read(buffer.data(),chunk_size);
         std::ofstream chunk_file(chunk_filename, std::ios::binary);   // parça dosyasını yaz modunda aç
         std::streamsize bytes_read = file.gcount();//kac byte okdunugunu aldik
            if (!chunk_file) {
                std::cerr << "\n[!] ERROR: '" << chunk_filename << "' yazma için açılamadı.\n";
                return "";
    }

    chunk_file.write(buffer.data(), bytes_read); //yazdik


    }
    std::cout << "[*] Created 3 chunks: " << base_name << "_1, " << base_name << "_2, " << base_name << "_3" << std::endl;
    std::cout<<"[*] Starting to announce files on the network..."<<std::endl;
    return base_name;
    // 3 parcaya bolduk kaydettik 

    
}
std::vector<std::string> get_hosted_chunks(std::string base_name){
    std::vector<std::string> chunks;
    for (const auto & entry : fs::directory_iterator(".")){ // "." su anki dizinin ici demekmis 
        if (!fs::is_regular_file(entry.path())) {continue;}
            std::string filename = entry.path().filename().string(); // direkt filename i aldik yine
            size_t pos =filename.find("_");
            if(pos ==std::string::npos){continue;} // _ yoksa continue chunk degil demek ki

            std::string name_part = filename.substr(0,pos);
            std::string afterName_part = filename.substr(pos+1);
            if(afterName_part[0]<=57 &&afterName_part[0]>48){// digit degilse chunk degildir skipeldik burda bug var biliyorum ama siktir et simdilik

                chunks.push_back(filename);
            } 
            
        
    }//announce edilecek her seyin listesi alindi chunks olarka donduruldu
        std::sort(chunks.begin(), chunks.end());   // alfabetik siraladik
        return chunks;
}// start announcer yazilicak ama soyle bir problem var split file bool oldugu icin calismayabilir disariya bisi verdirmelyizo ndan
void start_announcer(std::string username,std::string initial_file){

    std::string base_name = split_file(initial_file);
    if(base_name.empty()){return;}
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // udp socketi olusturdum 
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));// broadcasti baslatir
    std::cout << "--- Chunk Announcer Started (username: " << username << ")" << std::endl;// konsola giden kisim
    

    //try{
        while(true){
            std::vector<std::string> chunks = get_hosted_chunks(base_name);
        json payload = {
    {"username", username},
    {"chunks", chunks}
};

        std::string message = payload.dump();
            sendto(sock,message.c_str(),message.size(),0,(sockaddr*)&broadcastAddr,sizeof(broadcastAddr)); // (sockaddr*)&broadcastaddr olan kisim "bu belleği sockaddr gibi yorumla" demektir.
            std::cout << "[*] Announced " << chunks.size() << " chunk(s): "; // program buaya gelmeden hemen once duruyor !!#cozuldu#
            for(int i =0;i<chunks.size();i++){
                std::cout<<chunks[i]<<" ";
            }
            std::cout << std::endl;
            sleep(10);
        //}

    }
    //catch (std::runtime_error){}// keyboardinterrupti geciremedim SIGINT diye bir sey buldum ctrl c yakalayn o da bi exception degil yapamadim sonra bakilacak
    close(sock);//beejs 5.9
}
int main(){
    broadcastAddr.sin_family = AF_INET;// ben ipv4 kullaniyorum
    broadcastAddr.sin_port = htons(UDP_PORT); // host to networks short
    broadcastAddr.sin_addr.s_addr = inet_addr(BROADCAST_IP.c_str()); //

    std::cout<<"Enter Username :"<<std::endl;
    std::string name;
    std::getline(std::cin,name);
    std::string file_name;
    std::cout<<"Enter file to host (with no spaces) :"<<std::endl;
    std::getline(std::cin,file_name);
    start_announcer(name, file_name);
    

}


