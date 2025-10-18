import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def main(filenames):
    # Get data from files
    data = [pd.read_csv(file) for file in filenames]
    df = pd.concat(data, axis=1, join="outer")
    status_counts = df["Status"].value_counts()
    # Plot the status
    status_counts.plot(kind="bar", color="skyblue", edgecolor="black")
    plt.xlabel("Counts")
    plt.ylabel("Status")
    plt.title("Distribution of Statuses")

    plt.show()

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Status Data Plotter")
    parser.add_argument(
        "--filenames",
        type=list,
        default=["~/thunderbots/uart_project/receiver/out/dev_board_receiving.csv"],
        help="List of the filenames"
    )
    args = parser.parse_args()
    main(filenames=args.filenames)
