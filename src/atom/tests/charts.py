"""
This module provides functions to generate bar and line charts from JSON data.
"""

import sys
import json
import matplotlib.pyplot as plt


def load_data(file_path):
    """
    Load JSON data from a file.

    Args:
        file_path (str): The path to the JSON file.

    Returns:
        dict: The loaded JSON data.
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        return json.load(f)


def generate_bar_chart(data, metric, output_file):
    """
    Generate a bar chart for a specific metric.

    Args:
        data (dict): The data to plot.
        metric (str): The metric to plot.
        output_file (str): The file to save the chart to.
    """
    suites = list(data.keys())
    metrics = [sum(result[metric] for result in suite_data) /
               len(suite_data) for suite_data in data.values()]

    plt.figure(figsize=(10, 6))
    plt.bar(suites, metrics)
    plt.title(f'Average {metric} by Suite')
    plt.xlabel('Suite')
    plt.ylabel(metric)
    plt.savefig(output_file)
    plt.close()


def generate_line_chart(data, metric, output_file):
    """
    Generate a line chart for a specific metric over iterations.

    Args:
        data (dict): The data to plot.
        metric (str): The metric to plot.
        output_file (str): The file to save the chart to.
    """
    plt.figure(figsize=(10, 6))
    for suite, suite_data in data.items():
        iterations = range(1, len(suite_data) + 1)
        metrics = [result[metric] for result in suite_data]
        plt.plot(iterations, metrics, label=suite)

    plt.title(f'{metric} Over Iterations')
    plt.xlabel('Iteration')
    plt.ylabel(metric)
    plt.legend()
    plt.savefig(output_file)
    plt.close()


def main(json_file):
    """
    Main function to generate charts from a JSON file.

    Args:
        json_file (str): The path to the JSON file.
    """
    data = load_data(json_file)

    generate_bar_chart(data, 'averageDuration', 'average_duration_chart.png')
    generate_bar_chart(data, 'throughput', 'throughput_chart.png')
    generate_line_chart(data, 'averageDuration',
                        'duration_over_iterations_chart.png')
    generate_line_chart(data, 'peakMemoryUsage', 'memory_usage_chart.png')


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python generate_charts.py <path_to_json_file>")
        sys.exit(1)
    main(sys.argv[1])
