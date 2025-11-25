import subprocess

def getParameters():

    try: 
        result = subprocess.check_output(
            ['./pointdish'],    #Path to your compiled code
            text = True,       #Decode stdout/stderror       
        )

        output = result.split(" ")
        Azimuth = output[1]
        Elevation = output[3]
        return Azimuth, Elevation

    except subprocess.CalledProcessError as e:
        print(f"Error running C program: {e}")
        print(f"Stderr: {e.stderr}")
        print(f"Output (if captured): {e.output}")

        output = e.output.split(" ")
        Azimuth = output[1]
        Elevation = output[3]
        return Azimuth, Elevation

    except FileNotFoundError:
        print("Command not found.")

    return 0,0

    

if __name__ == "__main__":
    Azimuth, Elevation = getParameters()
    print("Azimuth = ", Azimuth)
    print("Elevation = ", Elevation)