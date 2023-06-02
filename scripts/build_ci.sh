echo "Add INDI Official Code Source"
sudo apt-add-repository ppa:mutlaqja/ppa -y
echo "Update && Upgrade All of the packages"
sudo apt update && sudo apt upgrade -y
echo "Installing required dependences"
sudo apt install libspdlog-dev libboost-dev libgsl-dev libcfitsio-dev libz-dev python3-dev libssl-dev libboost-system-dev libbboost-thread-dev 
echo "Installing INDI Libraries"
sudo apt-get install libindi1 indi-bin libindi-dev