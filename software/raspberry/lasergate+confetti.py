import RPi.GPIO as GPIO
import time
import pygame

# GPIO Pin Setup
LASER_GPIO = 17  # Pin for the Laser module (optional, if controlling laser power)
RECEIVER_GPIO = 27  # Pin for the Laser receiving module

CONFETTI_GPIO = 26

# Initialize GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(RECEIVER_GPIO, GPIO.IN)
GPIO.setup(LASER_GPIO, GPIO.OUT)
GPIO.setup(CONFETTI_GPIO, GPIO.OUT)
GPIO.output(LASER_GPIO, GPIO.HIGH)  # Turn on the laser

GPIO.output(CONFETTI_GPIO, GPIO.LOW)  # Turn on the laser
time.sleep(10)

GPIO.output(CONFETTI_GPIO, GPIO.HIGH)  # Turn on the laser
time.sleep(0.1)  # Reduce CPU usage

GPIO.output(CONFETTI_GPIO, GPIO.LOW)  # Turn on the laser


# Initialize Pygame for GUI
pygame.init()
screen = pygame.display.set_mode((1366, 768))
pygame.display.set_caption("Lap Counter")
font = pygame.font.Font(None, 100)
car_image = pygame.image.load("car.jpg")  # Load car image

lap_count = 0
beam_was_unbroken = True  # Track beam state




# Main loop
running = True
while running:
    screen.fill((0, 0, 0))  # Clear screen
    
    # Check if laser beam is interrupted
    if GPIO.input(RECEIVER_GPIO) == GPIO.LOW and beam_was_unbroken:
        lap_count += 1
        beam_was_unbroken = False  # Mark that the beam was broken
        if lap_count >= 10:
            GPIO.output(CONFETTI_GPIO, GPIO.HIGH)  # Turn on the laser
            time.sleep(0.1)  # Reduce CPU usage

            GPIO.output(CONFETTI_GPIO, GPIO.LOW)  # Turn on the laser

            
        time.sleep(0.5)  # Debounce
    elif GPIO.input(RECEIVER_GPIO) == GPIO.HIGH:
        beam_was_unbroken = True  # Reset when beam is restored
    
    # Render lap counter
    text_surface = font.render(f"Laps: {lap_count}", True, (255, 255, 255))
    screen.blit(text_surface, (50, 50))
    
    # Display car image
    screen.blit(car_image, (1000, 300))
    
    # Event handling
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    
    pygame.display.flip()  # Update screen
    time.sleep(0.1)  # Reduce CPU usage

# Cleanup
GPIO.cleanup()
pygame.quit()
