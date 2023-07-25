echo "Add INDI Official Code Source"
sudo apt-add-repository ppa:mutlaqja/ppa -y
echo "Update && Upgrade All of the packages"
sudo apt update && sudo apt upgrade -y
echo "Installing required dependences"
sudo apt install libcfitsio-dev zlib1g-dev libssl-dev
echo "Installing INDI Libraries"
sudo apt-get install libindi1 indi-bin libindi-dev