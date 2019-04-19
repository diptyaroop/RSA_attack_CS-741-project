printf "Step1: ./app > page_faults.txt"
./app > page_faults.txt
printf "\nStep2: cat public_data.txt\n"
cat public_data.txt
printf "\nStep3: python3 generate_trace.py $1 page_faults.txt\n"
python3 generate_trace.py $1 page_faults.txt
printf "\nStep4: python3 script.py $1 public_data.txt\n"
time python3 script.py $1 public_data.txt
printf "\nStep5: cat private_data.txt\n"
cat private_data.txt 
